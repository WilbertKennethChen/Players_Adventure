#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_primitives.h>
#include <functional>
#include <memory>
#include <string>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <nlohmann/json.hpp>

#include "Engine/AudioHelper.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Point.hpp"
#include "Engine/Resources.hpp"
#include "Player/GameData.hpp"
#include "PlayScene.hpp"
#include "LoginScene.hpp"
#include "UI/Component/ImageButton.hpp"
#include "UI/Component/Label.hpp"

void LoginScene::Initialize() {
    textBoxUser = false;
    textBoxPassword = false;

    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int centerX = w / 2;
    int startY = h / 4;

    const int boxWidth = 350;
    const int boxHeight = 50;
    const int spacing = 110; // More spacing between elements

    Engine::ImageButton* btn;
    AddNewObject(new Engine::Image("menu/main_menu.png", 0, 0, w, h, 0, 0));

    AddNewObject(new Engine::Label("Login/Register", "undertale.ttf", 60, centerX, 80, 255, 255, 255, 255, 0.5, 0.5));

    usernameBoxLabel = new Engine::Label("Username:", "undertale.ttf", 35, centerX - boxWidth / 2, startY - 30, 255, 255, 255, 255, 0.0, 0.5);
    passwordBoxLabel = new Engine::Label("Password:", "undertale.ttf", 35, centerX - boxWidth / 2, startY + spacing - 30, 255, 255, 255, 255, 0.0, 0.5);
    AddNewObject(usernameBoxLabel);
    AddNewObject(passwordBoxLabel);

    UsernameLabel = new Engine::Label("", "undertale.ttf", 24, centerX - boxWidth / 2 + 10, startY + 25, 255, 255, 255, 255, 0.0, 0.5);
    AddNewObject(UsernameLabel);
    PasswordLabel = new Engine::Label("", "undertale.ttf", 24, centerX - boxWidth / 2 + 10, startY + spacing + 25, 255, 255, 255, 255, 0.0, 0.5);
    AddNewObject(PasswordLabel);

    errorLabel = new Engine::Label("", "undertale.ttf", 20, centerX, startY + spacing * 2 - 30, 255, 0, 0, 255, 0.5, 0.5);
    AddNewObject(errorLabel);

    btn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", centerX - 200, startY + spacing * 2 + 10, 150, 60);
    btn->SetOnClickCallback(std::bind(&LoginScene::LoginOnClick, this));
    AddNewControlObject(btn);
    AddNewObject(new Engine::Label("Login", "undertale.ttf", 28, centerX - 125, startY + spacing * 2 + 40, 0, 0, 0, 255, 0.5, 0.5));

    btn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", centerX + 50, startY + spacing * 2 + 10, 150, 60);
    btn->SetOnClickCallback(std::bind(&LoginScene::RegisterOnClick, this));
    AddNewControlObject(btn);
    AddNewObject(new Engine::Label("Register", "undertale.ttf", 28, centerX + 125, startY + spacing * 2 + 40, 0, 0, 0, 255, 0.5, 0.5));

    btn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", centerX - 200, h - 100, 400, 80);
    btn->SetOnClickCallback(std::bind(&LoginScene::BackOnClick, this, 1));
    AddNewControlObject(btn);
    AddNewObject(new Engine::Label("Back", "undertale.ttf", 55, centerX, h - 65, 0, 0, 0, 255, 0.5, 0.5));

    int btnSize = 200;

    btn = new Engine::ImageButton("stage-select/skin.png", "stage-select/skinhovered.png", w - btnSize - 20, h - btnSize, btnSize, btnSize);
    btn->SetOnClickCallback(std::bind(&LoginScene::skinonclick, this));
    AddNewControlObject(btn);

    bgmInstance = AudioHelper::PlaySample("wii.mp3", true, AudioHelper::BGMVolume);

    std::string path = (std::filesystem::path(PROJECT_SOURCE_DIR) / "Resource/jsonData/SaveFile.json").string();
    std::ifstream jsonFileStream(path);
    nlohmann::json jsonData = nlohmann::json::parse(jsonFileStream);
    for (const auto& user : jsonData["Users"]) {
        userPass[user] = jsonData[user]["Pass"];
    }
    jsonFileStream.close();
}

void LoginScene::Draw() const {
    IScene::Draw();
    int centerX = Engine::GameEngine::GetInstance().GetScreenSize().x / 2;
    int startY = Engine::GameEngine::GetInstance().GetScreenSize().y / 4;
    int boxWidth = 350;
    int boxHeight = 50;
    int spacing = 110;

    ALLEGRO_COLOR boxColorUser = textBoxUser ? al_map_rgb(255, 255, 0) : al_map_rgb(255, 255, 255);
    ALLEGRO_COLOR boxColorPass = textBoxPassword ? al_map_rgb(255, 255, 0) : al_map_rgb(255, 255, 255);

    al_draw_rectangle(centerX - boxWidth / 2, startY, centerX + boxWidth / 2, startY + boxHeight, boxColorUser, 2);
    al_draw_rectangle(centerX - boxWidth / 2, startY + spacing, centerX + boxWidth / 2, startY + spacing + boxHeight, boxColorPass, 2);
}

void LoginScene::OnKeyDown(int keyCode) {
    IScene::OnKeyDown(keyCode);
    if (!textBoxUser && !textBoxPassword) return;

    auto handleInput = [](std::string& field, int key) {
        if (key == ALLEGRO_KEY_BACKSPACE && !field.empty()) {
            field.pop_back();
        } else if (field.length() < 13) {
            if (key >= ALLEGRO_KEY_A && key <= ALLEGRO_KEY_Z) field += 'a' + (key - ALLEGRO_KEY_A);
            else if (key >= ALLEGRO_KEY_0 && key <= ALLEGRO_KEY_9) field += '0' + (key - ALLEGRO_KEY_0);
        }
    };

    if (textBoxUser) {
        if (keyCode == ALLEGRO_KEY_ENTER) {
            textBoxUser = false;
            textBoxPassword = true;
            UsernameLabel->Text = Username;
            Password.clear();
            PasswordLabel->Text = "";
        } else {
            handleInput(Username, keyCode);
            UsernameLabel->Text = Username;
        }
    } else if (textBoxPassword) {
        if (keyCode == ALLEGRO_KEY_ENTER) {
            textBoxPassword = false;
        } else {
            handleInput(Password, keyCode);
            PasswordLabel->Text = std::string(Password.length(), '*');
        }
    }
}

void LoginScene::OnMouseDown(int button, int mx, int my) {
    IScene::OnMouseDown(button, mx, my);
    int centerX = Engine::GameEngine::GetInstance().GetScreenSize().x / 2;
    int startY = Engine::GameEngine::GetInstance().GetScreenSize().y / 4;
    int boxWidth = 350;
    int boxHeight = 50;
    int spacing = 110;

    textBoxUser = (mx >= centerX - boxWidth / 2 && mx <= centerX + boxWidth / 2 && my >= startY && my <= startY + boxHeight);
    textBoxPassword = (mx >= centerX - boxWidth / 2 && mx <= centerX + boxWidth / 2 && my >= startY + spacing && my <= startY + spacing + boxHeight);
}

void LoginScene::BackOnClick(int) {
    Engine::GameEngine::GetInstance().ChangeScene("start");
}

void LoginScene::LoginOnClick() {
    if (!Username.empty() && !Password.empty()) {
        if (userPass.find(Username) == userPass.end()) {
            errorLabel->Text = "User not found";
        } else if (userPass[Username] != Password) {
            errorLabel->Text = "Wrong password";
        } else {
            GameData::getInstance().Username = Username;
            Engine::GameEngine::GetInstance().ChangeScene("start");
        }
    }
}

void LoginScene::skinonclick(){
    Engine::GameEngine::GetInstance().ChangeScene("skin");
}


void LoginScene::RegisterOnClick() {
    if (!Username.empty() && !Password.empty()) {
        if (userPass.count(Username)) {
            errorLabel->Text = "Username already used";
        } else {
            GameData::getInstance().Username = Username;
            GameData::getInstance().ending = "";
            std::string path = (std::filesystem::path(PROJECT_SOURCE_DIR) / "Resource/jsonData/SaveFile.json").string();
            std::ifstream in(path);
            nlohmann::json jsonData = nlohmann::json::parse(in);
            in.close();
            jsonData["Users"].push_back(Username);
            jsonData[Username]["Pass"] = Password;
            jsonData[Username]["FriendlyEnemy"] = nlohmann::json::array();
            jsonData[Username]["DeadEnemy"] = nlohmann::json::array();
            std::ofstream out(path);
            out << jsonData.dump(4);
            out.close();
            Engine::GameEngine::GetInstance().ChangeScene("start");
        }
    }
}

void LoginScene::Terminate() {
    AudioHelper::StopSample(bgmInstance);
    bgmInstance = nullptr;
    IScene::Terminate();
}