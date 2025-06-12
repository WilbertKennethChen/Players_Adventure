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
#include <bits/ostream.tcc>

#include "Player/GameData.hpp"

void SkinScene::Initialize() {
    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int centerX = w / 2;

    AddNewObject(new Engine::Image("menu/main_menu.png", 0, 0, w, h, 0, 0));

    AddNewObject(new Engine::Label("Choose Your Skin", "undertale.ttf", 60, centerX, 80, 255, 255, 255, 255, 0.5, 0.5));


    // Back button like LoginScene
    Engine::ImageButton* btn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", centerX - 200, h - 100, 400, 80);
    btn->SetOnClickCallback(std::bind(&SkinScene::BackOnClick, this));
    AddNewControlObject(btn);
    AddNewObject(new Engine::Label("Back", "undertale.ttf", 55, centerX, h - 65, 0, 0, 0, 255, 0.5, 0.5));

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
     Engine::GameEngine::GetInstance().ChangeScene("start");
    });
    AddNewControlObject(selectBtn1);

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
    Engine::GameEngine::GetInstance().ChangeScene("start");
    });
    AddNewControlObject(selectBtn2);


    // OPTIONAL: Play music
    bgmInstance = AudioHelper::PlaySample("wii.mp3", true, AudioHelper::BGMVolume);
}

void SkinScene::BackOnClick() {
    Engine::GameEngine::GetInstance().ChangeScene("login");
}

void SkinScene::Terminate() {
    AudioHelper::StopSample(bgmInstance);
    bgmInstance = nullptr;
    IScene::Terminate();
}
