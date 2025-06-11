#include <allegro5/allegro_audio.h>
#include <functional>
#include <memory>
#include <string>

#include "Engine/AudioHelper.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Point.hpp"
#include "Engine/Resources.hpp"
#include "PlayScene.hpp"
#include "LoginScene.hpp"

#include <allegro5/allegro_primitives.h>

#include "Player/GameData.hpp"
#include "UI/Component/ImageButton.hpp"
#include "UI/Component/Label.hpp"
#include "UI/Component/Slider.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <filesystem>

void LoginScene::Initialize() {
    textBoxUser = false;
    textBoxPassword = false;

    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int centerX = w / 2;
    int startY = h / 4;

    const int boxWidth = 350;
    const int boxHeight = 50;
    const int spacing = 80;

    Engine::ImageButton* btn;

    // Title
    AddNewObject(new Engine::Label("Login/Register", "undertale.ttf", 48, centerX, startY - 60, 255, 255, 255, 255, 0.5, 0.5));

    // Input Labels
    UsernameLabel = new Engine::Label("", "pirulen.ttf", 24, centerX - boxWidth, startY + 10, 255, 255, 255, 255, 0.0, 0.0);
    AddNewObject(UsernameLabel);

    PasswordLabel = new Engine::Label("", "pirulen.ttf", 24, centerX - boxWidth, startY + spacing + 10, 255, 255, 255, 255, 0.0, 0.0);
    AddNewObject(PasswordLabel);

    // Login Button
    btn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", centerX - 170, startY + spacing * 2, 150, 60);
    btn->SetOnClickCallback(std::bind(&LoginScene::LoginOnClick, this));
    AddNewControlObject(btn);
    AddNewObject(new Engine::Label("Login", "pirulen.ttf", 28, centerX - 95, startY + spacing * 2 + 30, 0, 0, 0, 255, 0.5, 0.5));

    // Register Button
    btn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", centerX + 20, startY + spacing * 2, 150, 60);
    btn->SetOnClickCallback(std::bind(&LoginScene::RegisterOnClick, this));
    AddNewControlObject(btn);
    AddNewObject(new Engine::Label("Register", "pirulen.ttf", 28, centerX + 95, startY + spacing * 2 + 30, 0, 0, 0, 255, 0.5, 0.5));

    // Back Button (lower)
    btn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", centerX - 100, h - 120, 200, 60);
    btn->SetOnClickCallback(std::bind(&LoginScene::BackOnClick, this, 1));
    AddNewControlObject(btn);
    AddNewObject(new Engine::Label("Back", "pirulen.ttf", 28, centerX, h - 90, 0, 0, 0, 255, 0.5, 0.5));

    // Music
    bgmInstance = AudioHelper::PlaySample("select.ogg", true, AudioHelper::BGMVolume);

    //Get UserPass
    std::string path = (std::filesystem::path(PROJECT_SOURCE_DIR) / "Resource/jsonData/SaveFile.json").string();

    std::ifstream jsonFileStream(path);
    nlohmann::json jsonData = nlohmann::json::parse(jsonFileStream);
    int idx = 0;
    while (idx < jsonData["Users"].size()) {
        userPass[jsonData["Users"][idx]] = jsonData[jsonData["Users"][idx]]["Pass"];
        idx++;
    }
    jsonFileStream.close();
    // std::cout << userPass["nathan"] << std::endl;

}


void LoginScene::Draw() const {
    IScene::Draw();
    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int centerX = w / 2;
    int startY = h / 4;

    const int boxWidth = 350;
    const int boxHeight = 50;
    const int spacing = 80;

    ALLEGRO_COLOR boxColorUser = textBoxUser ? al_map_rgb(255, 255, 0) : al_map_rgb(255, 255, 255);
    ALLEGRO_COLOR boxColorPass = textBoxPassword ? al_map_rgb(255, 255, 0) : al_map_rgb(255, 255, 255);

    // Draw input boxes
    al_draw_rectangle(centerX - boxWidth / 2, startY, centerX + boxWidth / 2, startY + boxHeight, boxColorUser, 2);
    al_draw_rectangle(centerX - boxWidth / 2, startY + spacing, centerX + boxWidth / 2, startY + spacing + boxHeight, boxColorPass, 2);
}



void LoginScene::BackOnClick(int stage) {
    Engine::GameEngine::GetInstance().ChangeScene("start");
}

void LoginScene::OnKeyDown(int keyCode) {
    IScene::OnKeyDown(keyCode);
    if (!textBoxUser && !textBoxPassword) return;

    auto handleInput = [](std::string& field, int keyCode) {
        if (keyCode == ALLEGRO_KEY_BACKSPACE && !field.empty()) {
            field.pop_back();
        } else if (field.length() < 13) {
            if (keyCode >= ALLEGRO_KEY_A && keyCode <= ALLEGRO_KEY_Z) {
                field += 'a' + (keyCode - ALLEGRO_KEY_A);
            } else if (keyCode >= ALLEGRO_KEY_0 && keyCode <= ALLEGRO_KEY_9) {
                field += '0' + (keyCode - ALLEGRO_KEY_0);
            }
        }
    };

    if (textBoxUser) {
        if (keyCode == ALLEGRO_KEY_ENTER) {
            textBoxUser = false;
            textBoxPassword = true;
        } else {
            handleInput(Username, keyCode);
        }
        UsernameLabel->Text = Username;
    } else if (textBoxPassword) {
        if (keyCode == ALLEGRO_KEY_ENTER) {
            textBoxPassword = false;
        } else {
            handleInput(Password, keyCode);
        }
        PasswordLabel->Text = std::string(Password.length(), '*');
    }
}


void LoginScene::OnMouseDown(int button, int mx, int my) {
    IScene::OnMouseDown(button, mx, my);
    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int centerX = w / 2;
    int startY = h / 4;

    const int boxWidth = 350;
    const int boxHeight = 50;
    const int spacing = 80;

    textBoxUser = (mx >= centerX - boxWidth / 2 && mx <= centerX + boxWidth / 2 &&
                   my >= startY && my <= startY + boxHeight);

    textBoxPassword = (mx >= centerX - boxWidth / 2 && mx <= centerX + boxWidth / 2 &&
                       my >= startY + spacing && my <= startY + spacing + boxHeight);
}




void LoginScene::LoginOnClick() {
    if (!Username.empty() && !Password.empty()) {
        if ( userPass[Username] == Password) {
            GameData::getInstance().Username = Username;
            Engine::GameEngine::GetInstance().ChangeScene("start");
        } else {
            std::cout << userPass[Username] << std::endl;
            std::cout << "Wrong Password" << std::endl;
        }
    }
}
void LoginScene::RegisterOnClick() {
    if (!Username.empty() && !Password.empty()) {

        if (userPass.find(Username) != userPass.end()) {
            std::cout << "Used" << std::endl;
        } else {
            GameData::getInstance().Username = Username;
            std::string path = (std::filesystem::path(PROJECT_SOURCE_DIR) / "Resource/jsonData/SaveFile.json").string();
            std::ifstream jsonFileStream(path);
            nlohmann::json jsonData = nlohmann::json::parse(jsonFileStream);
            jsonFileStream.close();
            jsonData["Users"].push_back(Username);
            jsonData[Username]["Pass"] = Password;
            jsonData[Username]["FriendlyEnemy"] = nlohmann::json::array();
            jsonData[Username]["DeadEnemy"] = nlohmann::json::array();
            std::ofstream jsonOut(path);
            if (!jsonOut.is_open()) {
                std::cerr << "Failed to open SaveFile.json for writing!" << std::endl;
                return;
            }
            jsonOut << jsonData.dump(4);  // Add indent for readability
            jsonOut.close();
            Engine::GameEngine::GetInstance().ChangeScene("start");
        }
    }
}





void LoginScene::Terminate() {
    AudioHelper::StopSample(bgmInstance);
    bgmInstance = std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE>();
    IScene::Terminate();
}
