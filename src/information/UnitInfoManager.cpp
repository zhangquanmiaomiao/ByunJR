#include <sstream>

#include "ByunJRBot.h"
#include "common/Common.h"
#include "common/BotAssert.h"
#include "information/UnitInfoManager.h"
#include "util/Util.h"

UnitInfoManager::UnitInfoManager(ByunJRBot & bot)
    : m_bot(bot)
{

}

void UnitInfoManager::onStart()
{

}

void UnitInfoManager::onFrame()
{
    // If units are created or modified, update the unitInfo object.
    updateUnitInfo();

	//m_workers.onFrame();
    drawUnitInformation(100, 100);
    drawSelectedUnitDebugInfo();
}

// If units die, update the unitInfo object.
void UnitInfoManager::onUnitDestroyed(const sc2::Unit* unit)
{
    m_unitData[Util::GetPlayer(unit)].killUnit(unit);
}


void UnitInfoManager::updateUnitInfo()
{
    m_units[PlayerArrayIndex::Self].clear();
    m_units[PlayerArrayIndex::Enemy].clear();

    for (auto & unit : m_bot.Observation()->GetUnits())
    {
        if (Util::GetPlayer(unit) == PlayerArrayIndex::Self || Util::GetPlayer(unit) == PlayerArrayIndex::Enemy)
        {
            updateUnit(unit);
            m_units[Util::GetPlayer(unit)].push_back(unit);
        }        
    }

    // remove bad enemy units
    m_unitData[PlayerArrayIndex::Self].removeBadUnits();
    m_unitData[PlayerArrayIndex::Enemy].removeBadUnits();
}

const std::map<sc2::Tag, UnitInfo>& UnitInfoManager::getUnitInfoMap(const PlayerArrayIndex player) const
{
    return getUnitData(player).getUnitInfoMap();
}

const std::vector<const sc2::Unit*>& UnitInfoManager::getUnits(PlayerArrayIndex player) const
{
    BOT_ASSERT(m_units.find(player) != m_units.end(), "Couldn't find player units: %d", player);

    return m_units.at(player);
}

static std::string GetAbilityText(const sc2::AbilityID ability_id) {
    std::string str;
    str += sc2::AbilityTypeToName(ability_id);
    str += " (";
    str += std::to_string(uint32_t(ability_id));
    str += ")";
    return str;
}

void UnitInfoManager::drawSelectedUnitDebugInfo() const
{
    const sc2::Unit* unit = nullptr;
    for (const sc2::Unit* u : m_bot.Observation()->GetUnits())
    {
        if (u->is_selected && u->alliance == sc2::Unit::Self) {
            unit = u;
            break;
        }
    }

    if (!unit) { return; }

    auto debug = m_bot.Debug();
    auto query = m_bot.Query();
    auto abilities = m_bot.Observation()->GetAbilityData();

    std::string debug_txt = UnitTypeToName(unit->unit_type);
    if (debug_txt.length() < 1) 
    {
        debug_txt = "(Unknown name)";
        assert(0);
    }
    debug_txt += " (" + std::to_string(unit->unit_type) + ")";
        
    sc2::AvailableAbilities available_abilities = query->GetAbilitiesForUnit(unit);
    if (available_abilities.abilities.size() < 1) 
    {
        std::cout << "No abilities available for this unit" << std::endl;
    }
    else 
    {
        for (const sc2::AvailableAbility & available_ability : available_abilities.abilities) 
        {
            if (available_ability.ability_id >= abilities.size()) { continue; }

            const sc2::AbilityData & ability = abilities[available_ability.ability_id];

            debug_txt += GetAbilityText(ability.ability_id) + "\n";
        }
    }
    debug->DebugTextOut(debug_txt, unit->pos, sc2::Colors::Green);

    // Show the direction of the unit.
    sc2::Point3D p1; // Use this to show target distance.
    {
        const float length = 5.0f;
        sc2::Point3D p0 = unit->pos;
        p0.z += 0.1f; // Raise the line off the ground a bit so it renders more clearly.
        p1 = unit->pos;
        assert(unit->facing >= 0.0f && unit->facing < 6.29f);
        p1.x += length * std::cos(unit->facing);
        p1.y += length * std::sin(unit->facing);
        debug->DebugLineOut(p0, p1, sc2::Colors::Yellow);
    }

    // Box around the unit.
    {
        sc2::Point3D p_min = unit->pos;
        p_min.x -= 2.0f;
        p_min.y -= 2.0f;
        p_min.z -= 2.0f;
        sc2::Point3D p_max = unit->pos;
        p_max.x += 2.0f;
        p_max.y += 2.0f;
        p_max.z += 2.0f;
        debug->DebugBoxOut(p_min, p_max, sc2::Colors::Blue);
    }

    // Sphere around the unit.
    {
        sc2::Point3D p = unit->pos;
        p.z += 0.1f; // Raise the line off the ground a bit so it renders more clearly.
        debug->DebugSphereOut(p, 1.25f, sc2::Colors::Purple);
    }

    // Pathing query to get the target.
    bool has_target = false;
    sc2::Point3D target;
    std::string target_info;
    for (const sc2::UnitOrder& unit_order : unit->orders)
    {
        // TODO: Need to determine if there is a target point, no target point, or the target is a unit/snapshot.
        target.x = unit_order.target_pos.x;
        target.y = unit_order.target_pos.y;
        target.z = p1.z;
        has_target = true;

        target_info = "Target:\n";
        if (unit_order.target_unit_tag != 0x0LL) {
            target_info += "Tag: " + std::to_string(unit_order.target_unit_tag) + "\n";
        }
        if (unit_order.progress != 0.0f && unit_order.progress != 1.0f) {
            target_info += "Progress: " + std::to_string(unit_order.progress) + "\n";
        }

        // Perform the pathing query.
        {
            float distance = query->PathingDistance(unit->pos, target);
            target_info += "\nPathing dist: " + std::to_string(distance);
        }

        break;
    }

    if (has_target)
    {
        sc2::Point3D p = target;
        p.z += 0.1f; // Raise the line off the ground a bit so it renders more clearly.
        debug->DebugSphereOut(target, 1.25f, sc2::Colors::Blue);
        debug->DebugTextOut(target_info, p1, sc2::Colors::Yellow);
    }
}

// passing in a unit type of 0 returns a count of all units
size_t UnitInfoManager::getUnitTypeCount(const PlayerArrayIndex player, const sc2::UnitTypeID type, const bool completed) const
{
    size_t count = 0;

    for (auto & unit : getUnits(player))
    {
        if ((!type || type == unit->unit_type) && (!completed || unit->build_progress == 1.0f))
        {
            count++;
        }
    }

    return count;
}

void UnitInfoManager::drawUnitInformation(float x,float y) const
{
    if (!m_bot.Config().DrawEnemyUnitInfo)
    {
        return;
    }

    std::stringstream ss;

    // for each unit in the queue
    for (int t(0); t < 255; t++)
    {
        const int numUnits =      m_unitData.at(PlayerArrayIndex::Self).getNumUnits(t);
        const int numDeadUnits =  m_unitData.at(PlayerArrayIndex::Enemy).getNumDeadUnits(t);

        // if there exist units in the vector
        if (numUnits > 0)
        {
            ss << numUnits << "   " << numDeadUnits << "   " << sc2::UnitTypeToName(t) << std::endl;
        }
    }
    
    for (auto & kv : getUnitData(PlayerArrayIndex::Enemy).getUnitInfoMap())
    {
        m_bot.Debug()->DebugSphereOut(kv.second.lastPosition, 0.5f);
        m_bot.Debug()->DebugTextOut(sc2::UnitTypeToName(kv.second.type), kv.second.lastPosition);
    }
}

int UnitInfoManager::getNumAssignedWorkers(const sc2::Unit* depot)
{
	return m_unitData[PlayerArrayIndex::Self].getNumAssignedWorkers(depot);
}

void UnitInfoManager::setJob(const sc2::Unit* unit, const UnitMission job, const sc2::Tag jobUnitTag)
{
    m_unitData[Util::GetPlayer(unit)].setJob(unit, job, jobUnitTag);
}

void UnitInfoManager::setBuildingWorker(const sc2::Unit* worker, Building & b)
{
	m_unitData[Util::GetPlayer(worker)].setJob(worker, UnitMission::Build, b.type);
}

// This can only return your workers, not the enemy workers. 
std::set<const UnitInfo*> UnitInfoManager::getWorkers()
{
	return m_unitData[PlayerArrayIndex::Self].getWorkers();
}

const UnitInfo* UnitInfoManager::getUnitInfo(const sc2::Unit* unit)
{
	return &m_unitData[Util::GetPlayer(unit)].getUnitInfoMap().at(unit->tag);
}

void UnitInfoManager::updateUnit(const sc2::Unit* unit)
{
    if (!(Util::GetPlayer(unit) == PlayerArrayIndex::Self || Util::GetPlayer(unit) == PlayerArrayIndex::Enemy))
    {
        return;
    }

    m_unitData[Util::GetPlayer(unit)].updateUnit(unit);
}

// is the unit valid?
bool UnitInfoManager::isValidUnit(const sc2::Unit* unit)
{
    // we only care about our units and enemy units
    if (!(Util::GetPlayer(unit) == PlayerArrayIndex::Self || Util::GetPlayer(unit) == PlayerArrayIndex::Enemy))
    {
        return false;
    }

    // if it's a weird unit, don't bother
    if (unit->unit_type == sc2::UNIT_TYPEID::ZERG_EGG || unit->unit_type == sc2::UNIT_TYPEID::ZERG_LARVA)
    {
        return false;
    }

    // if the position isn't valid throw it out
    if (!m_bot.Map().isOnMap(unit->pos))
    {
        return false;
    }
    
    // s'all good baby baby
    return true;
}

void UnitInfoManager::getNearbyForce(std::vector<UnitInfo> & unitInfo, sc2::Point2D p, const PlayerArrayIndex player, const float radius) const
{
    bool hasBunker = false;
    // for each unit we know about for that player
    for (const auto & kv : getUnitData(player).getUnitInfoMap())
    {
        const UnitInfo & ui(kv.second);

        // if it's a combat unit we care about
        // and it's finished! 
        if (Util::IsCombatUnitType(ui.type) && Util::Dist(ui.lastPosition,p) <= radius)
        {
            // add it to the vector
            unitInfo.push_back(ui);
        }
    }
}

// Shorthand for the weird syntax required to get the unit data. This is available only inside this function. 
const UnitData & UnitInfoManager::getUnitData(const PlayerArrayIndex player) const
{
    return m_unitData.find(player)->second;
}

// getCombatUnits only has any meaning for your own units. 
// unitData does not publicly expose this function to prevent accidental requsting of the set of enemy combat units. 
std::set<const UnitInfo*> UnitInfoManager::getCombatUnits() const
{
	return getUnitData(PlayerArrayIndex::Self).GetCombatUnits();
}
