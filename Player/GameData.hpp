//
// Created by Celery on 6/6/2025.
//

#ifndef GAMESTATE_HPP
#define GAMESTATE_HPP
#include <string>
#include <unordered_map>
#include <Enemy/EnemyStruct.hpp>


class GameData {
public:
    bool spawned;
    bool changeMap;
    std::string currentEnemy;
    float playerHp;
    float playerMaxHp;
    float playerPosX;
    float playerPosY;

    std::unordered_map<std::string,EnemyStruct> enemyList;

    static GameData& getInstance() {
        static GameData instance;
        return instance;
    }

    GameData(const GameData&) = delete;
    void operator=(const GameData&) = delete;
private:
    GameData() = default;

};



#endif //GAMESTATE_HPP
