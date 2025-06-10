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
Engine::Label * labelNegotiate = nullptr;
Engine::Label * labelNegotiateInput = nullptr;
Engine::Image * imageSprite = nullptr;


void BattleScene::Initialize() {
    enemy = GameData::getInstance().currentEnemy;
    enemyName = enemy->getName();
    enemyMaxHp = GameData::getInstance().enemyList[enemyName].hp;
    img = GameData::getInstance().enemyList[enemyName].img;
    emotion = GameData::getInstance().enemyList[enemyName].emotion;
    desc = GameData::getInstance().enemyList[enemyName].desc;
    miniGame = GameData::getInstance().enemyList[enemyName].minigame;
    negotiateText = GameData::getInstance().enemyList[enemyName].negotiation;
    playerHp = GameData::getInstance().playerHp;
    playerMaxHp = GameData::getInstance().playerMaxHp;
    enemyHp = enemyMaxHp;

    //Init Groups
    AddNewControlObject(LabelGroup = new Group());
    AddNewControlObject(NotesGroup = new Group());
    AddNewControlObject(NegotiateGroup = new Group());
    AddNewControlObject(BoxGameGroup = new Group());

    idle = true;
    minigameNotesActive = false;
    minigameBoxActive = false;
    negotiateActive = false;
    convertNegotiateText();
    statusDisplayed = false;
    negotiationFailed = false;

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

    labelHpEnemy = new Engine::Label(buff, "undertale.ttf", 32, halfW, halfH/2 + 125, 255, 255, 255, 255, 0.5, 0.5);
    LabelGroup->AddNewObject(labelHpEnemy);
    Engine::ImageButton *btn;



    //player hp
    sprintf(buff,"HP:%d/%d",playerHp,playerMaxHp);
    labelHpPlayer = new Engine::Label(buff, "undertale.ttf", 32, 220, h-160, 255, 255, 255, 255, 0.5, 0.5);
    LabelGroup->AddNewObject(labelHpPlayer);

    //attack button
    btn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", 100, h-125, w/4, 100);
    btn->SetOnClickCallback(std::bind(&BattleScene::AttackOnClick, this));
    AddNewControlObject(btn);
    AddNewObject(new Engine::Label("Attack", "undertale.ttf", 50, 295, h-75, 0, 0, 0, 255, 0.5, 0.5));

    //negotiate button
    btn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", w/2 - w/8, h-125, w/4, 100);
    btn->SetOnClickCallback(std::bind(&BattleScene::NegotiateOnClick, this));
    AddNewControlObject(btn);
    AddNewObject(new Engine::Label("Negotiate", "undertale.ttf", 48, w/2 - w/8 + 200, h-75, 0, 0, 0, 255, 0.5, 0.5));

    //Run Button
    btn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", w - w/4-100, h-125, w/4, 100);
    btn->SetOnClickCallback(std::bind(&BattleScene::SpareOnClick, this));
    AddNewControlObject(btn);
    AddNewObject(new Engine::Label("Spare", "undertale.ttf", 50, w- w/4 + 100, h-75, 0, 0, 0, 255, 0.5, 0.5));

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
                updatePlayerHp(1);
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

            minigameNotesActive = false;
            enemyHit();
        }
    }

    if (minigameBoxActive) {
        notesTime += dt;
        BoxGameGroup->Update(dt);

        imageSprite->Position.x = mouseX;
        imageSprite->Position.y = mouseY;
        if (!circleActive) {
            timeSinceSpawn += dt;
            if (timeSinceSpawn >= spawnCD) {
                float randX = minigame_x1 + rand() % int((minigame_x2-minigame_x1) - 2 * circleRad);
                float randY = minigame_y1 + rand() % int((minigame_y2 - minigame_y1) - 2 * circleRad);
                circlePos = Engine::Point(randX, randY);
                circleActive = true;
                activeTime = 0.0f;
                timeSinceSpawn = 0.0f;
            }
        } else {
            activeTime += dt;
            float dx = mouseX - circlePos.x;
            float dy = mouseY - circlePos.y;


            if (activeTime >= spawnCD) {
                if (std::sqrt(dx*dx + dy*dy) > circleRad) {
                    updatePlayerHp(3);
                }
                circleActive = false;
                timeSinceSpawn = 0;
            }
        }



        if (notesTime >= 10) {
            al_show_mouse_cursor(al_get_current_display());
            minigameBoxActive = false;
            enemyHit();
            BoxGameGroup->Clear();
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
    if (negotiateActive) {
        std::cout << "negotiate" << std::endl;
        NegotiateGroup->Draw();
    }
    if (minigameBoxActive) {
        if (circleActive) {
            al_draw_circle(circlePos.x,circlePos.y,circleRad,al_map_rgba(255,255,0,255),2);
        }
        imageSprite->Draw();
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

    if (negotiateActive) {
        if (keyCode != negotiateKeys[inputPos]) {
            inputPos = 0;
            idle = true;
            negotiateActive = false;
            updatePlayerHp(3);
            NegotiateGroup->Clear();
            negotiationFailed = true;
        } else {
            labelNegotiateInput->Text = negotiateText.substr(0, inputPos+1);
            inputPos++;
        }

        if (inputPos == negotiateKeys.size()) {

            GameData::getInstance().friendlyEnemies.insert(enemyName);
            Engine::GameEngine::GetInstance().ChangeScene("play");
        }
    }




}

void BattleScene::setEnemyName(std::string name) {
    name = enemyName;
}

void BattleScene::AttackOnClick() {
    if (idle) {
        idle = false;
        if (miniGame == "notes") {
            if (!minigameNotesActive) {
                createNotesGame();
            }
        } else if (miniGame == "box") {
            if (!minigameBoxActive) {
                createBoxGame();
            }
        }
    }


    //enemy get damage (update enemy health)
}
void BattleScene::NegotiateOnClick() {
    if (idle) {
        if (!negotiateActive) {

            labelNegotiate = new Engine::Label(negotiateText, "undertale.ttf", 38, minigame_x1, minigame_y1 + 10, 255, 255, 255, 100, 0.5, 0.5,minigame_x2);
            labelNegotiateInput = new Engine::Label("", "undertale.ttf", 38, minigame_x1, minigame_y1 + 10, 255, 255, 0, 255, 0.5, 0.5, minigame_x2);

            NegotiateGroup->AddNewObject(labelNegotiate);
            NegotiateGroup->AddNewObject(labelNegotiateInput);
            keyStrokes.clear();
            negotiateActive = true;
            idle = false;
            inputPos = 0;
        }
    }
}

void BattleScene::SpareOnClick() {
    if (idle) {
        if (enemyHp == 5) {
            // std::cout << enemyName;
            GameData::getInstance().friendlyEnemies.insert(enemyName);
            Engine::GameEngine::GetInstance().ChangeScene("play");
        }
    }
}

void BattleScene::displayEnemyStatus() {
    char buff[20];
    // std::cout << "draw" << std::endl;

    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;
    int halfH = h / 2;

    labelDesc = new Engine::Label("~ " + desc + " ~", "undertale.ttf", 48, minigame_x2/2, minigame_y1 + 40, 255, 255, 255, 255, 0.5, 0.5);

    if (enemyHp == 5) emotion = "scared";
    labelEmo = new Engine::Label("~ " + enemyName + " is " + emotion + " ~", "undertale.ttf", 48, minigame_x2/2 , minigame_y1 +100, 255, 255, 255, 255, 0.5, 0.5);

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
    labelHpEnemy = new Engine::Label(buff, "undertale.ttf", 24, halfW, halfH/2 + 125, 255, 255, 255, 255, 0.5, 0.5);
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

void BattleScene::updatePlayerHp(int min){
    playerHp -= min;
    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;
    int halfH = h / 2;
    LabelGroup->RemoveObject(labelHpPlayer->GetObjectIterator());
    char buff[20];
    sprintf(buff,"HP:%d/%d",playerHp,playerMaxHp);
    labelHpPlayer = new Engine::Label(buff, "undertale.ttf", 30, 220, h-160, 255, 255, 255, 255, 0.5, 0.5);
    LabelGroup->AddNewObject(labelHpPlayer);
    if (playerHp <= 0) {
        al_show_mouse_cursor(al_get_current_display());
        Engine::GameEngine::GetInstance().ChangeScene("lose");
    }
}


void BattleScene::convertNegotiateText() {
    int pos=0;
    int n = negotiateText.length();
    while (pos < n) {
        char c = negotiateText[pos];
        switch (c) {
            case 'a' :
                negotiateKeys.push_back(ALLEGRO_KEY_A);
                break;
            case 'b' :
                negotiateKeys.push_back(ALLEGRO_KEY_B);
                break;
            case 'c' :
                negotiateKeys.push_back(ALLEGRO_KEY_C);
                break;
            case 'd' :
                negotiateKeys.push_back(ALLEGRO_KEY_D);
                break;
            case 'e' :
                negotiateKeys.push_back(ALLEGRO_KEY_E);
                break;
            case 'f' :
                negotiateKeys.push_back(ALLEGRO_KEY_F);
                break;
            case 'g' :
                negotiateKeys.push_back(ALLEGRO_KEY_G);
                break;
            case 'h' :
                negotiateKeys.push_back(ALLEGRO_KEY_H);
                break;
            case 'i' :
                negotiateKeys.push_back(ALLEGRO_KEY_I);
                break;
            case 'j' :
                negotiateKeys.push_back(ALLEGRO_KEY_J);
                break;
            case 'k' :
                negotiateKeys.push_back(ALLEGRO_KEY_K);
                break;
            case 'l' :
                negotiateKeys.push_back(ALLEGRO_KEY_L);
                break;
            case 'm' :
                negotiateKeys.push_back(ALLEGRO_KEY_M);
                break;
            case 'n' :
                negotiateKeys.push_back(ALLEGRO_KEY_N);
                break;
            case 'o' :
                negotiateKeys.push_back(ALLEGRO_KEY_O);
                break;
            case 'p' :
                negotiateKeys.push_back(ALLEGRO_KEY_P);
                break;
            case 'q' :
                negotiateKeys.push_back(ALLEGRO_KEY_Q);
                break;
            case 'r' :
                negotiateKeys.push_back(ALLEGRO_KEY_R);
                break;
            case 's' :
                negotiateKeys.push_back(ALLEGRO_KEY_S);
                break;
            case 't' :
                negotiateKeys.push_back(ALLEGRO_KEY_T);
                break;
            case 'u' :
                negotiateKeys.push_back(ALLEGRO_KEY_U);
                break;
            case 'v' :
                negotiateKeys.push_back(ALLEGRO_KEY_V);
                break;
            case 'w' :
                negotiateKeys.push_back(ALLEGRO_KEY_W);
                break;
            case 'x' :
                negotiateKeys.push_back(ALLEGRO_KEY_X);
                break;
            case 'y' :
                negotiateKeys.push_back(ALLEGRO_KEY_Y);
                break;
            case 'z' :
                negotiateKeys.push_back(ALLEGRO_KEY_Z);
                break;
            case '.' :
                negotiateKeys.push_back(ALLEGRO_KEY_FULLSTOP);
                break;
            case ',' :
                negotiateKeys.push_back(ALLEGRO_KEY_COMMA);
                break;
            case ' ' :
                negotiateKeys.push_back(ALLEGRO_KEY_SPACE);
                break;
        }
        pos++;
    }
}

void BattleScene::createBoxGame() {

    timeSinceSpawn = 0;
    activeTime = 0;
    circleActive = false;
    if (emotion == "calm") {
        circleRad = 25;
        spawnCD = 0.75;
        activeTime = 0.4;
    } else if (emotion == "scared") {
        circleRad = 35;
        spawnCD = 1;
        activeTime = 0.5;
    } else if (emotion == "furious") {
        circleRad = 15;
        spawnCD = 0.5;
        activeTime = 0.25;
    }

    idle = false;
    notesTime = 0;
    minigameBoxActive = true;
    imageSprite = new Engine::Image("play/bullet-8.png", mouseX, mouseY, 70, 70, 0.5, 0.5);
    BoxGameGroup->AddNewObject(imageSprite);
    al_hide_mouse_cursor(al_get_current_display());
}


void BattleScene::OnMouseMove(int mx, int my) {
    IScene::OnMouseMove(mx, my);
    if (minigameBoxActive) {
        mouseX = std::max(minigame_x1+10, std::min(minigame_x2-10, (float)mx));
        mouseY = std::max(minigame_y1+10, std::min(minigame_y2-10, (float)my));
        // std::cout << mouseX << " " << mouseY << std::endl;
    }

}

std::pair<int, int> BattleScene::generateRandomPos() const{
    srand(time(NULL));
    int x = rand() % ((int)minigame_x2-50);
    int y = (rand() % ((int) minigame_y2 - (int)minigame_y1)) + minigame_y1;
    return {x,y};
}






