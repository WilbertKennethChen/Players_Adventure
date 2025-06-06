//
// Created by Celery on 6/5/2025.
//

#ifndef BATTLEUI_HPP
#define BATTLEUI_HPP
#include <string>
#include <unordered_map>

#include "Engine/IScene.hpp"
#include "Engine/Group.hpp"


class BattleScene: public Engine::IScene {
protected:
    std::string enemyName;


public:
    bool done = false;
    // std::unordered_map<std::string,>;
    int enemyMaxHp;
    int enemyHp;
    std::string img;

    float playerMaxHp;
    float playerHp;



    explicit BattleScene() = default;
    void Initialize() override;
    void setEnemyName(std::string name);
    void Update();
    void Draw() const override;
    void OnKeyDown(int keyCode) override;
    void AttackOnClick();
    void NegotiateOnClick();
    void RunOnClick();
};




#endif //BATTLEUI_HPP
