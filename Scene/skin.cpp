//
// Created by swift on 6/12/2025.
//
#include <allegro5/allegro_audio.h>
#include <functional>
#include "skin.h"

#include <iostream>

#include "Engine/AudioHelper.hpp"
#include "Engine/GameEngine.hpp"
#include "UI/Component/Image.hpp"
#include "UI/Component/ImageButton.hpp"
#include "UI/Component/Label.hpp"
#include <memory>
// #include <bits/ostream.tcc>

#include "Player/GameData.hpp"

void SkinScene::Initialize() {
    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int centerX = w / 2;
    std::string selectedSkin = GameData::getInstance().playerSkin;

    AddNewObject(new Engine::Image("menu/main_menu.png", 0, 0, w, h, 0, 0));

    AddNewObject(new Engine::Label("Choose Your Skin", "undertale.ttf", 60, centerX, 80, 255, 255, 255, 255, 0.5, 0.5));


    // Back button like LoginScene
    Engine::ImageButton* btn = new Engine::ImageButton("stage-select/ui_button.png", "stage-select/ui_button_hovered.png", centerX - 127, h - 150, 250, 80);
    btn->SetOnClickCallback(std::bind(&SkinScene::BackOnClick, this));
    AddNewControlObject(btn);
    AddNewObject(new Engine::Label("Back", "undertale.ttf", 55, centerX, h - 110, 0, 0, 0, 255, 0.5, 0.5));

    AddNewObject(new Engine::Image("play/player_idle_0.png", centerX - 380, 250, 256, 256));
    AddNewObject(new Engine::Label("Tuyul", "undertale.ttf", 36, centerX - 252, 220, 255, 255, 255, 255, 0.5, 0.5));

    // First skin select button
    Engine::ImageButton* selectBtn1 = new Engine::ImageButton(
        "stage-select/selectnormal.png", "stage-select/select.png",
        centerX - 400, 530, 280, 160
    );

    selectBtn1->SetOnClickCallback([]() {
     GameData::getInstance().playerSkin = "play/silent_wanderer_";
        std::cout << "tuyul ready" << std::endl;
     Engine::GameEngine::GetInstance().ChangeScene("login");
    });
    AddNewControlObject(selectBtn1);

    if (selectedSkin == "play/silent_wanderer_") {
        AddNewObject(new Engine::Label("Selected", "undertale.ttf", 28, centerX - 252, 180, 255, 215, 0, 255, 0.5, 0.5));
    }


    // Second skin image
    AddNewObject(new Engine::Image("poci/kuo_idle/kuo1.png", centerX + 150, 235,  192, 320));
    AddNewObject(new Engine::Label("Mr.Kuo", "undertale.ttf", 36, centerX + 250, 220, 255, 255, 255, 255, 0.5, 0.5));


    // Second skin select button
    Engine::ImageButton* selectBtn2 = new Engine::ImageButton(
        "stage-select/selectnormal.png", "stage-select/select.png",
        centerX + 104, 530, 280, 160 // +104 = -400 + 280 + padding (approx. 504)
    );
    selectBtn2->SetOnClickCallback([]() {
    GameData::getInstance().playerSkin = "play/kuo";
        std::cout << "kuo ready" << std::endl;
    Engine::GameEngine::GetInstance().ChangeScene("login");
    });
    AddNewControlObject(selectBtn2);

    if (selectedSkin == "play/kuo") {
        AddNewObject(new Engine::Label("Selected", "undertale.ttf", 28, centerX + 250, 180, 255, 215, 0, 255, 0.5, 0.5));
    }



    // OPTIONAL: Play music
    bgmInstance = AudioHelper::PlaySample("wii.mp3", true, AudioHelper::BGMVolume);
}

void SkinScene::BackOnClick() {
    Engine::GameEngine::GetInstance().ChangeScene("start");
}

void SkinScene::Terminate() {
    AudioHelper::StopSample(bgmInstance);
    bgmInstance = nullptr;
    IScene::Terminate();
}
