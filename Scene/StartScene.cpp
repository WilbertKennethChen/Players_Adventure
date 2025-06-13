//
// Created by Hsuan on 2024/4/10.
//

#include "StartScene.h"
#include <allegro5/allegro_audio.h>
#include <functional>
#include <memory>
#include <string>
#include <cmath>

#include "Engine/AudioHelper.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Point.hpp"
#include "Engine/LOG.hpp"
#include "Engine/Resources.hpp"
#include "PlayScene.hpp"
#include "Scene/StartScene.h"

#include "Enemy/Enemy.hpp"
#include "nlohmann/json.hpp"
#include "Player/GameData.hpp"
#include "UI/Component/ImageButton.hpp"
#include "UI/Component/Label.hpp"
#include "UI/Component/Slider.hpp"

// TODO HACKATHON-2 (1/3): You can imitate the 2 files: 'StartScene.hpp', 'StartScene.cpp' to implement your SettingsScene.
void StartScene::Initialize() {
    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;
    int halfH = h / 2;

    //initialized player state
    GameData::getInstance().playerMaxHp = 20;
    GameData::getInstance().playerHp = 20;
    GameData::getInstance().playerPosX = halfW;
    GameData::getInstance().playerPosY = halfH;
    GameData::getInstance().currentEnemy = nullptr;
    GameData::getInstance().spawned = false;
    GameData::getInstance().changeMap = false;

    //Add new enemy
    GameData::getInstance().enemyList["enemy1"] = EnemyStruct{"enemy1",25,"play/archivist_shade","calm","Regular ghost, doesn't have any friend","box", "forgive me if i intrude on your resting place, noble spirit. i offer no threat, only passage, and perhaps, companionship on this lonely path."};
    GameData::getInstance().enemyList["groveDad"] = EnemyStruct{"groveDad",25,"play/grove_warden","calm","A father trying his best", "notes","wise protector, i sense your deep love for your kin. i seek only safe passage, promising to respect your domain and pose no threat to your precious family. may our paths cross peacefully, and your children continue to thrive."};
    GameData::getInstance().enemyList["groveChild"] = EnemyStruct{"groveChild",15,"play/Grove_child","scared","A clueless viny creature, looks young","notes", "hello, little one, you have such beautiful colors. i'm just passing through and wanted to admire your lovely home."};
    GameData::getInstance().enemyList["jesterKnight"] = EnemyStruct{"jesterKnight",35,"play/jester_knight","calm","A knight that have guarded this place for too long","box","brave knight, your vigil is long and honorable. i seek only to pass this place, not to disturb your sacred duty or the peace you've so valiantly maintained through the ages. grant me safe passage, and your tireless watch shall remain undisturbed, and i will offer a prayer for your steadfast soul."};

    std::string instructionText = "Click the tunnel to start...";
    float fontSize = 36;
    Engine::ImageButton *btn;

    // ADD THIS LINE: Set the bonfire background for the main menu.
    AddNewObject(new Engine::Image("menu/main_menu.png", 0, 0, w, h, 0, 0));


    btn = new Engine::ImageButton("play/dirty-3.png", "stage-select/emptynew.png", halfW - 250, halfH / 2, 400);
    btn->SetOnClickCallback(std::bind(&StartScene::PlayOnClick, this, 1));400,
    AddNewControlObject(btn);

    btn = new Engine::ImageButton("stage-select/settingok.png", "stage-select/settinggud.png", halfW + 490, halfH * 3 / 2 + 25, 200, 200);
    btn->SetOnClickCallback(std::bind(&StartScene::SettingsOnClick, this, 2));
    AddNewControlObject(btn);

    btn = new Engine::ImageButton("stage-select/champ.png", "stage-select/champione.png", halfW + 620, halfH * 3 / 2 + 25, 200, 200);
    btn->SetOnClickCallback(std::bind(&StartScene::LeaderboardOnClick, this,3));
    AddNewControlObject(btn);

    btn = new Engine::ImageButton("stage-select/icon1.png", "stage-select/icon2.png", halfW + 360, halfH * 3 / 2 + 25, 200, 200);
    btn->SetOnClickCallback(std::bind(&StartScene::LoginOnClick, this));
    AddNewControlObject(btn);

    unsigned char initialAlpha = 128;
    instructionLabel = new Engine::Label(instructionText, "undertale.ttf", fontSize,
                                   halfW - 45, h - 40,
                                   255, 255, 255, initialAlpha,
                                   0.5, 1.0);

    std::string titleText = "??? Adventure";
    std::string user = GameData::getInstance().Username;

    if (user != "") {
        user[0]-=32;
        titleText = user + "'s Adventure";
        AddNewObject(instructionLabel);
    } else {
        AddNewObject(new Engine::Label("Login or Register to Start", "undertale.ttf", fontSize,
                               halfW - 45, h - 90,
                               255, 255, 255, 100,
                               0.5, 1.0));
    }
    AddNewObject(new Engine::Label(titleText, "undertale.ttf", 120, halfW, halfH / 3 - 100, 255, 255, 255, 255, 0.5, 0.5)); // Adjusted color to be visible


    Engine::LOG(Engine::INFO) << "PreStartScene instruction label added.";
    animationTime = 0.0f;
    bgmInstance = AudioHelper::PlaySample("opening.ogg", true, AudioHelper::BGMVolume);
}

void StartScene::Update(float deltaTime) {
    IScene::Update(deltaTime);

    animationTime += deltaTime;

    float pulseSpeed = 1.5f;
    float minAlpha = 25.0f;
    float maxAlpha = 100.0f;

    float normalizedSin = (sin(animationTime * pulseSpeed) + 1.0f) / 2.0f;
    float currentAlphaFloat = minAlpha + normalizedSin * (maxAlpha - minAlpha);

    currentAlphaFloat = std::max(0.0f, std::min(255.0f, currentAlphaFloat));
    unsigned char currentAlphaByte = static_cast<unsigned char>(currentAlphaFloat);

    if (instructionLabel) {
        instructionLabel->Color = al_map_rgba(255, 255, 255, currentAlphaByte);
    }
}
void StartScene::Terminate() {
    instructionLabel = nullptr;
    AudioHelper::StopSample(bgmInstance);
    bgmInstance = nullptr;
    IScene::Terminate();
}
void StartScene::PlayOnClick(int stage) {
    if (GameData::getInstance().Username != "") {
        Engine::GameEngine::GetInstance().ChangeScene("stage-select");
    }

}
void StartScene::SettingsOnClick(int stage) {
    Engine::GameEngine::GetInstance().ChangeScene("settings");
}
void StartScene::LeaderboardOnClick(int stage)
{
    Engine::GameEngine::GetInstance().ChangeScene("leaderboard");
}

void StartScene::LoginOnClick() {
    Engine::GameEngine::GetInstance().ChangeScene("login");
}
