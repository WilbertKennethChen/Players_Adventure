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
    bool idle;
    float minigame_x1;
    float minigame_x2;
    float minigame_y1;
    float minigame_y2;

public:
    bool done = false;
    // std::unordered_map<std::string,>;
    int enemyMaxHp;
    int enemyHp;
    std::string img;
    std::string emotion;
    std::string desc;
    std::string miniGame;
    int playerMaxHp;
    int playerHp;

    Group * LabelGroup;


    explicit BattleScene() = default;
    void Initialize() override;
    void setEnemyName(std::string name);
    void Update(float dt) override;
    void Draw() const override;
    void OnKeyDown(int keyCode) override;
    void AttackOnClick();
    void NegotiateOnClick();
    void RunOnClick();
    void displayEnemyStatus();
    void deleteEnemyStatus();
};




#endif //BATTLEUI_HPP
