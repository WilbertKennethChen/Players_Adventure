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
    imgTitle = {
    "Proposal Draft 5/27", "Map Idea", "Scene Idea", "Start Scene 6/8", "Early Player Sprite", "Early Dialogue", "Map 1 6/9",
        "Player Sprite 6/12", "Dialogue 6/13"
    };
    std::vector<std::string>images = {
    "credit1","credit3","credit4","credit5","credit6",
        "credit7","credit8","credit9","credit10"
    };
    if (GameData::getInstance().ending == "good") {
        for (int i =0;i<texts.size();i++) {
            Engine::Label * label;
            label = new Engine::Label(texts[i],"undertale.ttf",32,
                halfW, scrollY+10,255,255,255,255,0.5,0.5);
            CreditGroup->AddNewObject(label);
            credits.push_back(label);
        }

        for (int i = 0;i<images.size();i++) {
            Engine::Image * img;
            img = new Engine::Image("play/"+images[i]+".jpeg",halfW,scrollY+75,600,400,0.5,0.5);
            Engine::Label * label;
            label  = new Engine::Label(imgTitle[i],"undertale.ttf",32,
                halfW, scrollY+10,255,255,255,175,0.5,0.5);

            CreditGroup->AddNewObject(img);
            CreditGroup->AddNewObject(label);
            creditsImg.push_back(img);
            creditsImgTitle.push_back(label);
        }
        // Engine::Image * img;
        // img = new Engine::Image("play/team.jpeg",halfW,scrollY+75,500,600,0.5,0.5);
        // Engine::Label * label;
        // label  = new Engine::Label("The Team","undertale.ttf",32,
        //     halfW, scrollY+10,255,255,255,175,0.5,0.5);
        // creditsImg.push_back(img);
        // CreditGroup->AddNewObject(img);
        // creditsImgTitle.push_back(label);
        // CreditGroup->AddNewObject(label);


    } else {
        Done = true;
    }


    // if (GameData::getInstance().ending != "good") {
    //     Done = true;
    // } else {
    //     for (auto& img : creditsImg) {
    //         img->Position.y = scrollY + 500;
    //     }
    // }
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

        float spacingLabel = 60.0f;
        int idx = 0;
        for (size_t i = 0; i < credits.size(); ++i) {
            credits[i]->Position.y = scrollY + idx * spacingLabel;
            idx++;
        }
        float spacingImg = 450;
        for (size_t i =0; i<creditsImg.size(); ++i) {
            creditsImg[i]->Position.y = scrollY + (idx * 60) + 200 + (i * spacingImg);
            creditsImgTitle[i]->Position.y = creditsImg[i]->Position.y + 250;
            idx++;
        }

        // optional: when the last line has fully scrolled off, go back to menu
        float totalHeight = (credits.size() * 60) + (creditsImg.size() * spacingImg) + 450;
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
            std::string desc;
            std::string descImg;
            int r,g,b;
            if (GameData::getInstance().ending == "bad") {
                title = "Bad End";
                desc = ":(";
                descImg = "fail.png";
                r = 255;
                g = 0;
                b = 0;
            } else if (GameData::getInstance().ending == "good") {
                title = "True End";
                desc = "Thank you for Playing";
                descImg = "team.png";
                r=0;
                g = 255;
                b = 0;
            } else if (GameData::getInstance().ending == "normal") {
                desc = "Good Job";
                descImg = "normal.jpeg";
                r = 0;
                g = 0;
                b = 255;
                title = "Normal End";
            }
            bgmInstance = AudioHelper::PlaySample("edsfx.mp3",false,AudioHelper::BGMVolume);
            AddNewObject(new Engine::Label(title, "undertale.ttf", 72, halfW, 100 , r, g, b, 255, 0.5, 0.5));
            AddNewObject(new Engine::Image("play/"+descImg,halfW,halfH,0,0,0.5,0.5));
            AddNewObject(new Engine::Label(desc, "undertale.ttf", 48, halfW, halfH + 250 , 255, 255, 255, 255, 0.5, 0.5));
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
        std::cout << GameData::getInstance().Username  << " " << jsonData[GameData::getInstance().Username]["Ending"] << std::endl;
        if (jsonData[GameData::getInstance().Username]["Ending"] == "") {
            std::cout << "saved to leaderboard" << std::endl;
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
        std::string path = (std::filesystem::path(PROJECT_SOURCE_DIR) / "Resource/jsonData/SaveFile.json").string();
        std::ifstream jsonFileStream(path);
        nlohmann::json jsonData = nlohmann::json::parse(jsonFileStream);
        jsonFileStream.close();
        jsonData[GameData::getInstance().Username]["DeadEnemy"] = GameData::getInstance().deadEnemies;
        jsonData[GameData::getInstance().Username]["FriendlyEnemy"] = GameData::getInstance().friendlyEnemies;
        jsonData[GameData::getInstance().Username]["Ending"] = GameData::getInstance().ending;
        jsonData[GameData::getInstance().Username]["Score"] = GameData::getInstance().score;
        std::ofstream jsonOut(path);
        jsonOut << jsonData.dump(4);
        jsonOut.close();
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
    creditsImg.clear();
    creditsImgTitle.clear();
}
