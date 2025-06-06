//
// Created by Celery on 6/5/2025.
//

#include "BattleScene.hpp"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

#include "Engine/GameEngine.hpp"
#include "Player/Player.hpp"
#include "UI/Component/Image.hpp"
#include "UI/Component/Label.hpp"
#include "Player/GameData.hpp"
#include "UI/Component/ImageButton.hpp"
Player* player;
void BattleScene::Initialize() {

    enemyName = GameData::getInstance().currentEnemy;
    enemyMaxHp = GameData::getInstance().enemyList[enemyName].hp;
    img = GameData::getInstance().enemyList[enemyName].img;
    enemyHp = enemyMaxHp;

    //
    // std::cout << enemyName << " " << enemyMaxHp << " " << enemyDamage << " " << img << std::endl;





    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;
    int halfH = h / 2;
    AddNewObject(new Engine::Image("lose/benjamin-happy.png", halfW, halfH/2 - 100, 300, 300, 0.5, 0.5));
    AddNewObject(new Engine::Label("Enemy Hp", "pirulen.ttf", 24, halfW, halfH - 200, 255, 255, 255, 255, 0.5, 0.5));
    Engine::ImageButton *btn;

    //attack button
    btn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", 100, h-100, w/4, 100);
    btn->SetOnClickCallback(std::bind(&BattleScene::AttackOnClick, this));
    AddNewControlObject(btn);
    AddNewObject(new Engine::Label("Attack", "pirulen.ttf", 48, 100, h-100, 0, 0, 0, 255, 0.5, 0.5));

    //negotiate button
    btn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", w/2, h-100, w/4, 100);
    btn->SetOnClickCallback(std::bind(&BattleScene::NegotiateOnClick, this));
    AddNewControlObject(btn);
    AddNewObject(new Engine::Label("Negotiate", "pirulen.ttf", 48, w/2, h-100, 0, 0, 0, 255, 0.5, 0.5));

    //Run Button
    btn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", w - w/4, h-100, w/4, 100);
    btn->SetOnClickCallback(std::bind(&BattleScene::RunOnClick, this));
    AddNewControlObject(btn);
    AddNewObject(new Engine::Label("Run", "pirulen.ttf", 48, w- w/4, h-100, 0, 0, 0, 255, 0.5, 0.5));
}



void BattleScene::Update() {

}

void BattleScene::Draw() const {
    IScene::Draw();
}

void BattleScene::OnKeyDown(int keyCode) {
    IScene::OnKeyDown(keyCode);
    if (keyCode == ALLEGRO_KEY_ESCAPE) {
        Engine::GameEngine::GetInstance().ChangeScene("play");
    }
}

void BattleScene::setEnemyName(std::string name) {
    name = enemyName;
}

void BattleScene::AttackOnClick() {

}
void BattleScene::NegotiateOnClick() {

}

void BattleScene::RunOnClick() {

}



