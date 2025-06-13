//
// Created by Celery on 6/12/2025.
//
#include <allegro5/allegro_audio.h>
#include <functional>
#include <memory>
#include <string>

#include "Engine/AudioHelper.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Point.hpp"
#include "Engine/Resources.hpp"
#include "PlayScene.hpp"
#include "StageSelectScene.hpp"
#include "Player/GameData.hpp"
#include <iostream>

#include "Player/GameData.hpp"
#include "UI/Component/ImageButton.hpp"
#include "UI/Component/Label.hpp"
#include "UI/Component/Slider.hpp"
#include "CreditScene.hpp"

#include <filesystem>
#include <iterator>
#include <allegro5/allegro_primitives.h>

#include "Engine/GameEngine.hpp"
#include <nlohmann/json.hpp>
#include <fstream>

#include "LeaderboardScene.hpp"
#include "UI/Component/Label.hpp"

void CreditScene::Initialize() {
    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;
    int halfH = h / 2;
    scrollY = h;
    scrollSpeed = 50;
    Done = false;
    Skip = false;
    toggleSpeed = false;
    revealStarted = false;
    fadeDuration = 1;
    fadeA = 0;

    AddNewObject(CreditGroup = new Group());

    std::vector<std::string>texts = {"Director: lorem",
        "Lead Programmer: ipsum",
        "Art & Design: dolor",
        "Music: sit",
        "sit: Amet",
        "Prof: Kuo"};

    for (int i =0;i<texts.size();i++) {
        Engine::Label * label;
        label = new Engine::Label(texts[i],"undertale.ttf",32,
            halfW, scrollY+10,255,255,255,255,0.5,0.5);
        CreditGroup->AddNewObject(label);
        credits.push_back(label);
    }
    if (GameData::getInstance().ending != "good") {
        Done = true;
    }
    std::cout << Done << std::endl;

}

void CreditScene::Draw() const {
    IScene::Draw();
    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    if (revealStarted && fadeA < 255.0f) {
        al_draw_filled_rectangle(
            0, 0, w, h,
            al_map_rgba(0, 0, 0, static_cast<int>(255 - fadeA))
        );
    }
}



void CreditScene::Update(float deltaTime) {
    IScene::Update(deltaTime);
    // slide everything upward
    if (!Done && !Skip) {
        scrollY -= scrollSpeed * deltaTime;

        float spacing = 60.0f;
        for (size_t i = 0; i < credits.size(); ++i) {
            credits[i]->Position.y = scrollY + i * spacing;
        }

        // optional: when the last line has fully scrolled off, go back to menu
        float totalHeight = credits.size() * spacing;
        if (scrollY + totalHeight < 0) {
            // Engine::GameEngine::GetInstance().ChangeScene("start");
            Done = true;
            CreditGroup->Clear();
            revealStarted = true;
        }
    }
    if (Done && fadeA < 255) {
        fadeA += (255/fadeDuration) * deltaTime;
        if (fadeA >=255) {
            fadeA = 255;

            int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
            int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
            int halfW = w / 2;
            int halfH = h / 2;
            std::string title;
            int r,g,b;
            if (GameData::getInstance().ending == "bad") {
                title = "Bad End";
                r = 255;
                g = 0;
                b = 0;
            } else if (GameData::getInstance().ending == "good") {
                title = "True End";
                r=0;
                g = 255;
                b = 0;
            } else if (GameData::getInstance().ending == "normal") {
                r = 0;
                g = 0;
                b = 255;
                title = "Normal End";
            }
            bgmInstance = AudioHelper::PlaySample("edsfx.mp3",false,AudioHelper::BGMVolume);
            AddNewObject(new Engine::Label(title, "undertale.ttf", 72, halfW, halfH /2 , r, g, b, 255, 0.5, 0.5));
            AddNewObject(new Engine::Label("Press Enter to Return to Main Menu / Press E to Reset", "undertale.ttf", 24, halfW, h - 40 , 255, 255, 255, 125, 0.5, 0.5));

            Done = false;
            Skip = true;
        }
    }

    if (!toggleSpeed) {
        scrollSpeed = 50;
    } else {
        scrollSpeed = 1000;
    }

}


void CreditScene::OnKeyDown(int keyCode) {
    IScene::OnKeyDown(keyCode);
    if (keyCode == ALLEGRO_KEY_TAB) {
        toggleSpeed = !toggleSpeed;
    }
    if (Skip && keyCode == ALLEGRO_KEY_ENTER) {
        std::string path = (std::filesystem::path(PROJECT_SOURCE_DIR) / "Resource/jsonData/SaveFile.json").string();
        std::ifstream jsonFileStream(path);
        nlohmann::json jsonData = nlohmann::json::parse(jsonFileStream);
        jsonFileStream.close();
        if (jsonData[GameData::getInstance().Username]["Ending"].empty()) {
            jsonData[GameData::getInstance().Username]["DeadEnemy"] = GameData::getInstance().deadEnemies;
            jsonData[GameData::getInstance().Username]["FriendlyEnemy"] = GameData::getInstance().friendlyEnemies;
            jsonData[GameData::getInstance().Username]["Ending"] = GameData::getInstance().ending;
            jsonData[GameData::getInstance().Username]["Score"] = GameData::getInstance().score;
            std::ofstream jsonOut(path);
            jsonOut << jsonData.dump(4);
            jsonOut.close();
            std::cout << "back" << std::endl;
            path = (std::filesystem::path(PROJECT_SOURCE_DIR) / "Resource/scoreboard.txt").string();
            LeaderboardScene::AddScoreToLeaderboard(GameData::getInstance().Username,
                                                    GameData::getInstance().score,
                                                    path,
                                                    GameData::getInstance().ending);
        }
        Engine::GameEngine::GetInstance().ChangeScene("start");
    }
    if (Skip && keyCode == ALLEGRO_KEY_E) {
        GameData::getInstance().deadEnemies.clear();
        GameData::getInstance().friendlyEnemies.clear();
        GameData::getInstance().ending = "";
        GameData::getInstance().score = 0;
        std::cout << "reset" << std::endl;
        Engine::GameEngine::GetInstance().ChangeScene("start");
    }
}


void CreditScene::BackOnClick() {
    Engine::GameEngine::GetInstance().ChangeScene("start");
}


void CreditScene::Terminate() {
    IScene::Terminate();
    CreditGroup->Clear();
    credits.clear();
}
