#pragma once

#include "Common.h"
#include "MicroManager.h"

class ByunJRBot;

class MeleeManager: public MicroManager
{

public:

    MeleeManager(ByunJRBot & bot);
    void    executeMicro(const std::vector<UnitTag> & targets);
    void    assignTargets(const std::vector<UnitTag> & targets);
    int     getAttackPriority(const UnitTag & attacker, const UnitTag & unit);
    UnitTag getTarget(const UnitTag & meleeUnit, const std::vector<UnitTag> & targets);
    bool    meleeUnitShouldRetreat(const UnitTag & meleeUnit, const std::vector<UnitTag> & targets);
};
