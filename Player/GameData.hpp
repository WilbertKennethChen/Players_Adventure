//
// Created by Celery on 6/6/2025.
//

#ifndef GAMESTATE_HPP
#define GAMESTATE_HPP
#include <set>
#include <string>
#include <unordered_map>
#include <Enemy/EnemyStruct.hpp>
#include <NPC/Npc.hpp>


class GameData {
public:
    bool spawned;
    bool changeMap;
    Npc* currentEnemy;
    float playerHp;
    float playerMaxHp;
    float playerPosX;
    float playerPosY;
    std::string Username = "";
    std::string ending;

    std::unordered_map<std::string,EnemyStruct> enemyList;
    std::set<std::string> deadEnemies;
    std::set<std::string> friendlyEnemies;
    static GameData& getInstance() {
        static GameData instance;
        return instance;
    }
    std::string playerSkin = "play/silent_wanderer_";

    GameData(const GameData&) = delete;
    void operator=(const GameData&) = delete;
private:
    GameData() = default;

};



#endif //GAMESTATE_HPP
