//
// Created by Celery on 6/5/2025.
//

#ifndef BATTLEUI_HPP
#define BATTLEUI_HPP
#include <queue>
#include <string>
#include <vector>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <random>

#include "Engine/IScene.hpp"
#include "Engine/Group.hpp"
#include <allegro5/allegro_audio.h>


class BattleScene: public Engine::IScene {

private:
    mutable std::mt19937 rng;
    mutable std::uniform_int_distribution<int> distX;
    mutable std::uniform_int_distribution<int> distY;

protected:
    std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE> bgmInstance;
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
    bool introAnimationPlaying = true;
    float introRadius = 0.0f;
    float introMaxRadius = 500.0f;
    float introSpeed = 800.0f;

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
    void SetupBattleUI();
    bool battleUIInitialized = false;
    ALLEGRO_BITMAP* enemyBitmap = nullptr;
    float enemyDrawSize = 0.0f;
    float playerHitTimer = 0.0f;
    float enemyShakeTime = 0.0f;
    float enemyShakeDuration = 0.3f;
    float enemyShakeMagnitude = 10.0f;

    std::vector<ALLEGRO_BITMAP*> enemyFrames;
    int currentEnemyFrame = 0;
    float frameTime = 0.0f;
    float frameInterval = 0.3f; // seconds per frame

    //notes game
    void createNotesGame();
    void readNotesWave();
    void enemyHit();


    //negotiate
    void convertNegotiateText();

    //box game
    void createBoxGame();
    std::pair<int,int> generateRandomPos()const;

    struct HitEffect {
        float x, y;
        float timeLeft;
        bool isBox = false;
    };

    std::list<HitEffect> hitEffects;

};




#endif //BATTLEUI_HPP
