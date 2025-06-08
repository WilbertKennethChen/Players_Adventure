//
// Created by Celery on 6/5/2025.
//

#include "BattleScene.hpp"
#include <fstream>
#include <iostream>
#include <allegro5/allegro_primitives.h>
#include <nlohmann/json.hpp>

#include "Engine/GameEngine.hpp"
#include "Player/Player.hpp"
#include "UI/Component/Image.hpp"
#include "UI/Component/Label.hpp"
#include "UI/Component/Notes.hpp"
#include "Player/GameData.hpp"
#include "UI/Component/ImageButton.hpp"

Player* player;
Npc *enemy;

bool statusDisplayed;
Engine::Label * labelDesc = nullptr;
Engine::Label * labelEmo = nullptr;
Engine::Label * labelHpEnemy = nullptr;
Engine::Label * labelHpPlayer = nullptr;

void BattleScene::Initialize() {
    enemy = GameData::getInstance().currentEnemy;
    enemyName = enemy->getName();
    enemyMaxHp = GameData::getInstance().enemyList[enemyName].hp;
    img = GameData::getInstance().enemyList[enemyName].img;
    emotion = GameData::getInstance().enemyList[enemyName].emotion;
    desc = GameData::getInstance().enemyList[enemyName].desc;
    miniGame = GameData::getInstance().enemyList[enemyName].minigame;

    playerHp = GameData::getInstance().playerHp;
    playerMaxHp = GameData::getInstance().playerMaxHp;
    enemyHp = enemyMaxHp;

    //Init Groups
    AddNewControlObject(LabelGroup = new Group());
    AddNewControlObject(NotesGroup = new Group());


    idle = true;
    minigameNotesActive = false;
    statusDisplayed = false;

    //
    // std::cout << enemyName << " " << enemyMaxHp << " " << enemyDamage << " " << img << std::endl;





    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;
    int halfH = h / 2;


    //Enemy
    AddNewObject(new Engine::Image(img, halfW, halfH/2 - 50, 300, 300, 0.5, 0.5));

    char buff[20];
    sprintf(buff,"HP:%d/%d",enemyHp,enemyMaxHp);

    labelHpEnemy = new Engine::Label(buff, "pirulen.ttf", 24, halfW, halfH/2 + 125, 255, 255, 255, 255, 0.5, 0.5);
    LabelGroup->AddNewObject(labelHpEnemy);
    Engine::ImageButton *btn;



    //player hp
    sprintf(buff,"HP:%d/%d",playerHp,playerMaxHp);
    labelHpPlayer = new Engine::Label(buff, "pirulen.ttf", 30, 220, h-160, 255, 255, 255, 255, 0.5, 0.5);
    LabelGroup->AddNewObject(labelHpPlayer);

    //attack button
    btn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", 100, h-125, w/4, 100);
    btn->SetOnClickCallback(std::bind(&BattleScene::AttackOnClick, this));
    AddNewControlObject(btn);
    AddNewObject(new Engine::Label("Attack", "pirulen.ttf", 48, 295, h-75, 0, 0, 0, 255, 0.5, 0.5));

    //negotiate button
    btn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", w/2 - w/8, h-125, w/4, 100);
    btn->SetOnClickCallback(std::bind(&BattleScene::NegotiateOnClick, this));
    AddNewControlObject(btn);
    AddNewObject(new Engine::Label("Negotiate", "pirulen.ttf", 40, w/2 - w/8 + 200, h-75, 0, 0, 0, 255, 0.5, 0.5));

    //Run Button
    btn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", w - w/4-100, h-125, w/4, 100);
    btn->SetOnClickCallback(std::bind(&BattleScene::RunOnClick, this));
    AddNewControlObject(btn);
    AddNewObject(new Engine::Label("Run", "pirulen.ttf", 48, w- w/4 + 100, h-75, 0, 0, 0, 255, 0.5, 0.5));

    //minigame screen
    minigame_x1 = 10;
    minigame_x2 = w-10;
    minigame_y1 = halfH/2 + 150;
    minigame_y2 = h-200;

}



void BattleScene::Update(float dt) {
    // std::cout << idle << std::endl;
    NotesGroup->Update(dt);
    if (idle && !statusDisplayed) {
        displayEnemyStatus();
    }
    if (!idle && statusDisplayed) {
        deleteEnemyStatus();
    }


    //Notes minigame update
    if (minigameNotesActive && !notesWave.empty()) {
        //spawn notes;

        notesTime += dt;

        for (int lane = 0;lane<3;lane++) {
            while (!notesWave[lane].empty() && notesWave[lane].front() <= notesTime) {
                notesWave[lane].pop();
                if (lane == 0) {
                    NotesGroup->AddNewObject(new Notes(minigame_x1 + 10, minigame_y1 + 45,lane,notesSpeed));
                } else if (lane == 1) {
                    NotesGroup->AddNewObject(new Notes(minigame_x1 + 10, (minigame_y1 + minigame_y2)/2,lane,notesSpeed));
                } else if (lane == 2) {
                    NotesGroup->AddNewObject(new Notes(minigame_x1 + 10, minigame_y2 - 45,lane,notesSpeed));
                }


            }
        }






        const auto& notes = NotesGroup->GetObjects();
        std::list<IObject*>remove;
        for (auto& note : NotesGroup->GetObjects()) {
            //delete if outside screen
            if (note->Position.x >= Engine::GameEngine::GetInstance().GetScreenSize().x) {
                remove.push_back(note);
                playerHp-=1;
                updatePlayerHp();
            }
            // mark as can be targeted
            Notes * insideNote = dynamic_cast<Notes*>(note);
            if (note->Position.x >= minigame_x2 - 300 && note->Position.x <= minigame_x2 - notesGap) {
                insideNote->hit = true;
            } else {
                insideNote->hit = false;
            }
        }
        for (auto note : remove) {
            NotesGroup->RemoveObject(note->GetObjectIterator());
        }
        if (NotesGroup->GetObjects().empty() && notesWave[0].empty() && notesWave[1].empty() && notesWave[2].empty()) {
            std::cout << "empty notes" << std::endl;
            minigameNotesActive = false;
            enemyHit();
        }
    }
}

void BattleScene::Draw() const {
    IScene::Draw();
    LabelGroup->Draw();

    al_draw_rectangle(minigame_x1, minigame_y1, minigame_x2, minigame_y2, al_map_rgba(255,255,255,255), 2);
    if (minigameNotesActive) {
        NotesGroup->Draw();


        al_draw_rectangle(minigame_x2 - 300,minigame_y1,minigame_x2-notesGap,minigame_y2,al_map_rgba(255,255,0,255),1);
    }
}

void BattleScene::OnKeyDown(int keyCode) {
    IScene::OnKeyDown(keyCode);
    if (minigameNotesActive) {
        if (keyCode == ALLEGRO_KEY_J) {
            IObject* remove = nullptr;
            for (auto& note : NotesGroup->GetObjects()) {
                Notes * insideNote = dynamic_cast<Notes*>(note);
                if (insideNote->hit && insideNote->lane == 0) {
                    remove = note;
                    break;
                }
            }
            if (remove) {
                NotesGroup->RemoveObject(remove->GetObjectIterator());
            }
        }

        if (keyCode == ALLEGRO_KEY_K) {
            IObject* remove = nullptr;
            for (auto& note : NotesGroup->GetObjects()) {
                Notes * insideNote = dynamic_cast<Notes*>(note);
                if (insideNote->hit && insideNote->lane == 1) {
                    remove = note;
                    break;
                }
            }
            if(remove) {
                NotesGroup->RemoveObject(remove->GetObjectIterator());
            }

        }

        if (keyCode == ALLEGRO_KEY_L) {
            IObject* remove= nullptr;
            for (auto& note : NotesGroup->GetObjects()) {
                Notes * insideNote = dynamic_cast<Notes*>(note);
                if (insideNote->hit && insideNote->lane == 2) {
                    remove = note;
                    break;
                }
            }
            if (remove) {
                NotesGroup->RemoveObject(remove->GetObjectIterator());
            }

        }

    }


}

void BattleScene::setEnemyName(std::string name) {
    name = enemyName;
}

void BattleScene::AttackOnClick() {
    idle = false;
    if (!minigameNotesActive) {
        createNotesGame();
    }


    //enemy get damage (update enemy health)
}
void BattleScene::NegotiateOnClick() {
    idle = true;
}

void BattleScene::RunOnClick() {
    if (enemyHp == 5) {
        std::cout << enemyName;
        GameData::getInstance().friendlyEnemies.insert(enemyName);
        Engine::GameEngine::GetInstance().ChangeScene("play");

    }
}

void BattleScene::displayEnemyStatus() {
    char buff[20];
    // std::cout << "draw" << std::endl;

    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;
    int halfH = h / 2;

    labelDesc = new Engine::Label("~ " + desc + " ~", "pirulen.ttf", 30, minigame_x2/2, minigame_y1 + 40, 255, 255, 255, 255, 0.5, 0.5);

    if (enemyHp == 5) emotion = "scared";
    labelEmo = new Engine::Label("~ " + enemyName + " is " + emotion + " ~", "pirulen.ttf", 30, minigame_x2/2 , minigame_y1 +100, 255, 255, 255, 255, 0.5, 0.5);

    LabelGroup->AddNewObject(labelEmo);
    LabelGroup->AddNewObject(labelDesc);
    statusDisplayed = true;
    //Notes minigame
    if (emotion == "calm") {
        notesGap = 175;
        notesSpeed = 500;
    }
    if (emotion == "scared") {
        notesGap = 150;
        notesSpeed = 300;
    }
    if (emotion == "furious") {
        notesGap = 200;
        notesSpeed = 700;
    }
}

void BattleScene::deleteEnemyStatus() {
    LabelGroup->RemoveObject(labelDesc->GetObjectIterator());
    LabelGroup->RemoveObject(labelEmo->GetObjectIterator());
    statusDisplayed = false;
}

void BattleScene::createNotesGame() {
    minigameNotesActive = true;
    readNotesWave();
    notesTime=0;

}


void BattleScene::enemyHit() {
    enemyHp = std::max(enemyHp-5,0);
    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;
    int halfH = h / 2;
    LabelGroup->RemoveObject(labelHpEnemy->GetObjectIterator());
    char buff[20];
    sprintf(buff,"HP:%d/%d",enemyHp,enemyMaxHp);
    labelHpEnemy = new Engine::Label(buff, "pirulen.ttf", 24, halfW, halfH/2 + 125, 255, 255, 255, 255, 0.5, 0.5);
    LabelGroup->AddNewObject(labelHpEnemy);


    if (enemyHp <= 0) {
        GameData::getInstance().deadEnemies.insert(enemyName);
        Engine::GameEngine::GetInstance().ChangeScene("play");
    }
    idle = true;
}

void BattleScene::readNotesWave() {
    notesWave.clear();
    std::ifstream fin("Resource/notes1.txt");
    float lane0, lane1, lane2;
    notesWave.resize(3);
    while (fin >> lane0 && fin >> lane1 && fin >> lane2) {
        std::cout << lane0;
        notesWave[0].push(lane0);
        notesWave[1].push(lane1);
        notesWave[2].push(lane2);
    }
    fin.close();
}

void BattleScene::updatePlayerHp(){
    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;
    int halfH = h / 2;
    LabelGroup->RemoveObject(labelHpPlayer->GetObjectIterator());
    char buff[20];
    sprintf(buff,"HP:%d/%d",playerHp,playerMaxHp);
    labelHpPlayer = new Engine::Label(buff, "pirulen.ttf", 30, 220, h-160, 255, 255, 255, 255, 0.5, 0.5);
    LabelGroup->AddNewObject(labelHpPlayer);
}




