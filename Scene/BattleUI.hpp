//
// Created by Celery on 6/5/2025.
//

#ifndef BATTLEUI_HPP
#define BATTLEUI_HPP
#include <string>

#include "Engine/IScene.hpp"
#include "Engine/Group.hpp"


class BattleUI: public Engine::IScene {
protected:
    std::string enemyName;


public:
    bool done = false;
    int maxHp;
    int hp;
    std::string img;
    int damage;



    BattleUI(std::string enemyName);
    void Update();
    void Draw() const override;
    void OnKeyDown(int keyCode) override;
};



#endif //BATTLEUI_HPP
