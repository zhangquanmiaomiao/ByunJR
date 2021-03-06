#pragma once
#include <sc2api/sc2_api.h>

struct BotConfig
{
    std::string RawConfigString;

    bool ConfigFileFound;
    bool ConfigFileParsed;
    std::string ConfigFileLocation;
        
    bool UsingAutoObserver;        
    
    std::string BotName;
    std::string MapName;
    std::string Authors;
    bool PrintInfoOnStart;
    std::string BotMode;
    
    std::string StrategyName;
    std::string ReadDir;
    std::string WriteDir;
    bool TrainingMode;
    bool UseEnemySpecificStrategy;
    bool FoundEnemySpecificStrategy; 
    
    bool DrawGameInfo;
    bool DrawTileInfo;
    bool DrawBaseLocationInfo;
    bool DrawWalkableSectors;
    bool DrawResourceInfo;
    bool DrawProductionInfo;
    bool DrawScoutInfo;
    bool DrawWorkerInfo;
    bool DrawModuleTimers;
    bool DrawReservedBuildingTiles;
    bool DrawBuildingInfo;
    bool DrawEnemyUnitInfo;
    bool DrawLastSeenTileInfo;
    bool DrawUnitTargetInfo;
    bool DrawSquadInfo;        
    
    sc2::Color ColorLineTarget;
    sc2::Color ColorLineMineral;
    sc2::Color ColorUnitNearEnemy;
    sc2::Color ColorUnitNotNearEnemy;
    
    bool KiteWithRangedUnits;    
    bool ScoutHarassEnemy;
    int CombatUnitsForAttack;
    
    int BuildingSpacing;
    int ProxyLocationX;
    int ProxyLocationY;
 
    BotConfig();

    void ReadConfigFile();
    void SetProxyLocation(const int x, const int y);
};