//
// Created by Celery on 6/5/2025.
//

#ifndef BATTLEUI_HPP
#define BATTLEUI_HPP
#include <queue>
#include <string>
#include <vector>
#include <iostream>
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

    bool minigameNotesActive;
public:

    // std::unordered_map<std::string,>;
    int enemyMaxHp;
    int enemyHp;
    std::string img;
    std::string emotion;
    std::string desc;
    std::string miniGame;
    int playerMaxHp;
    int playerHp;
    //notes miniGame
    float notesTime;
    int notesGap;
    int notesSpeed;
    std::vector<std::queue<float>> notesWave;
    Group * LabelGroup;
    Group * NotesGroup;

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
    void updatePlayerHp();
    //notes game
    void createNotesGame();
    void readNotesWave();
    void enemyHit();

};




#endif //BATTLEUI_HPP
