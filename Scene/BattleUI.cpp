//
// Created by Celery on 6/5/2025.
//

#include "BattleUI.hpp"
#include <fstream>
#include <nlohmann/json.hpp>

#include "Engine/GameEngine.hpp"

BattleUI::BattleUI(std::string enemyName): enemyName(enemyName) {
    std::ifstream jsonFileStream("Resource/jsonData/enemyInfo.json");
    nlohmann::json jsonData = nlohmann::json::parse(jsonFileStream);
    maxHp = jsonData[enemyName]["hp"];
    damage = jsonData[enemyName]["damage"];
    img = jsonData[enemyName]["img"];
    hp = maxHp;
    jsonFileStream.close();
}



void BattleUI::Update() {

}

void BattleUI::Draw() const {
    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;
    int halfH = h / 2;

    // AddNewObject(new Engine::Image("lose/benjamin-happy.png", halfW, halfH, 0, 0, 0.5, 0.5));
    // AddNewObject(new Engine::Label("You Lose :(", "pirulen.ttf", 48, halfW, halfH / 4 + 10, 255, 255, 255, 255, 0.5, 0.5));

}

void BattleUI::OnKeyDown(int keyCode) {
    IScene::OnKeyDown(keyCode);
}
