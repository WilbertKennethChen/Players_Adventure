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
    float mouseX;
    float mouseY;
    bool minigameNotesActive;
    bool negotiateActive;
    bool minigameBoxActive;
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

    //negotiate
    std::string negotiateText;
    std::vector<int> negotiateKeys;
    std::list<int> keyStrokes;
    int inputPos;
    bool negotiationFailed;

    //boxGame
    bool drawBox;
    Engine::Point circlePos;
    float timeSinceSpawn;
    float spawnCD;
    float activeTime;
    float circleRad;
    bool circleActive;


    Group * LabelGroup;
    Group * NotesGroup;
    Group * NegotiateGroup;
    Group * BoxGameGroup;

    explicit BattleScene() = default;
    void Initialize() override;
    void setEnemyName(std::string name);
    void Update(float dt) override;
    void Draw() const override;
    void OnKeyDown(int keyCode) override;
    void OnMouseMove(int mx, int my) override;
    void AttackOnClick();
    void NegotiateOnClick();
    void SpareOnClick();
    void displayEnemyStatus();
    void deleteEnemyStatus();
    void updatePlayerHp(int min);

    //notes game
    void createNotesGame();
    void readNotesWave();
    void enemyHit();


    //negotiate
    void convertNegotiateText();

    //box game
    void createBoxGame();
    std::pair<int,int> generateRandomPos()const;

};




#endif //BATTLEUI_HPP
