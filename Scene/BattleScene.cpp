//
// Created by Celery on 6/5/2025.
//

#include <allegro5/allegro_audio.h>
#include "BattleScene.hpp"
#include <fstream>
#include <iostream>
#include <allegro5/allegro_primitives.h>
#include <nlohmann/json.hpp>
#include "Engine/AudioHelper.hpp"
#include <random>
#include <chrono>

#include "Engine/GameEngine.hpp"
#include "Player/Player.hpp"
#include "UI/Component/Image.hpp"
#include "UI/Component/Label.hpp"
#include "UI/Component/Notes.hpp"
#include "Player/GameData.hpp"
#include "UI/Component/ImageButton.hpp"
#include "Engine/Resources.hpp"

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
    idle = false;
    battleUIInitialized = false;
    introAnimationPlaying = true;
    introRadius = 0;
    playerHitTimer = 0.0f;
     int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;
    int halfH = h / 2;

    // Then set minigame bounds
    minigame_x1 = 10;
    minigame_x2 = w - 10;
    minigame_y1 = halfH / 2 + 150;
    minigame_y2 = h - 200;

    // ✅ Finally: seed RNG and define distributions
    std::random_device rd;
    auto timeSeed = static_cast<unsigned>(std::chrono::system_clock::now().time_since_epoch().count());
    std::seed_seq seed{rd(), timeSeed};
    rng = std::mt19937(seed);
    distX = std::uniform_int_distribution<int>(minigame_x1, minigame_x2 - 50);
    distY = std::uniform_int_distribution<int>(minigame_y1, minigame_y2);

    labelHpEnemy = nullptr;
    labelHpPlayer = nullptr;
    labelDesc = nullptr;
    labelEmo = nullptr;
    labelNegotiate = nullptr;
    labelNegotiateInput = nullptr;
    imageSprite = nullptr;
    enemy = GameData::getInstance().currentEnemy;
    enemyName = enemy->getName();
    enemyMaxHp = GameData::getInstance().enemyList[enemyName].hp;
    img = "Resource/images/" + GameData::getInstance().enemyList[enemyName].img;
    emotion = GameData::getInstance().enemyList[enemyName].emotion;
    desc = GameData::getInstance().enemyList[enemyName].desc;
    miniGame = GameData::getInstance().enemyList[enemyName].minigame;
    negotiateText = GameData::getInstance().enemyList[enemyName].negotiation;
    playerHp = GameData::getInstance().playerHp;
    playerMaxHp = GameData::getInstance().playerMaxHp;
    enemyHp = enemyMaxHp;
    for (int i = 1; i <= 5; ++i) {
        std::string path = img + "_" + std::to_string(i) + ".png";
        ALLEGRO_BITMAP* bmp = al_load_bitmap(path.c_str());
        if (bmp) enemyFrames.push_back(bmp);
        else std::cerr << "Failed to load: " << path << std::endl;
    }

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

    bgmInstance = AudioHelper::PlaySample("cancan.mp3", true, AudioHelper::BGMVolume);
}

void BattleScene::SetupBattleUI() {
    if (battleUIInitialized) return;
    battleUIInitialized = true;

    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;
    int halfH = h / 2;

    char buff[20];

    // Enemy HP Label
    sprintf(buff,"HP:%d/%d",enemyHp,enemyMaxHp);
    labelHpEnemy = new Engine::Label(buff, "undertale.ttf", 32, halfW - 250, halfH/2 + 125, 255, 255, 255, 255, 0.5, 0.5);
    LabelGroup->AddNewObject(labelHpEnemy);

    // Player HP Label
    sprintf(buff,"HP:%d/%d",playerHp,playerMaxHp);
    labelHpPlayer = new Engine::Label(buff, "undertale.ttf", 32, 290, h - 160,
                                      255, 255, 255, 255, 0.5, 0.5);
    LabelGroup->AddNewObject(labelHpPlayer);

    // Attack Button
    auto* btn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png",
                                        100, h - 125, w / 4, 100);
    btn->SetOnClickCallback(std::bind(&BattleScene::AttackOnClick, this));
    AddNewControlObject(btn);
    AddNewObject(new Engine::Label("Attack", "undertale.ttf", 50, 295, h - 75,
                                   0, 0, 0, 255, 0.5, 0.5));

    // Negotiate Button
    btn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png",
                                  w / 2 - w / 8, h - 125, w / 4, 100);
    btn->SetOnClickCallback(std::bind(&BattleScene::NegotiateOnClick, this));
    AddNewControlObject(btn);
    AddNewObject(new Engine::Label("Negotiate", "undertale.ttf", 48, w / 2 - w / 8 + 200, h - 75,
                                   0, 0, 0, 255, 0.5, 0.5));

    // Spare Button
    btn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png",
                                  w - w / 4 - 100, h - 125, w / 4, 100);
    btn->SetOnClickCallback(std::bind(&BattleScene::SpareOnClick, this));
    AddNewControlObject(btn);
    AddNewObject(new Engine::Label("Spare", "undertale.ttf", 50, w - w / 4 + 100, h - 75,
                                   0, 0, 0, 255, 0.5, 0.5));
}




void BattleScene::Update(float dt) {
    // std::cout << idle << std::endl;
    if (introAnimationPlaying) {
        introRadius += introSpeed * dt;
        if (introRadius >= introMaxRadius) {
            introRadius = introMaxRadius;
            introAnimationPlaying = false;
            SetupBattleUI(); // Show UI now
            displayEnemyStatus();
            idle = true;
        }
        return;
    }
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
    for (auto it = hitEffects.begin(); it != hitEffects.end(); ) {
        it->timeLeft -= dt;
        if (it->timeLeft <= 0) {
            it = hitEffects.erase(it);
        } else {
            ++it;
        }
    }

    if (minigameBoxActive) {
        notesTime += dt;
        BoxGameGroup->Update(dt);

        imageSprite->Position.x = mouseX;
        imageSprite->Position.y = mouseY;
        if (notesTime >= 10) {
            // If a circle is still active, wait for its timeout
            if (circleActive) {
                activeTime += dt;
                float dx = mouseX - circlePos.x;
                float dy = mouseY - circlePos.y;

                if (activeTime >= spawnCD) {
                    if (std::sqrt(dx * dx + dy * dy) <= circleRad) {
                        hitEffects.push_back({circlePos.x, circlePos.y, 0.3f, true});
                    }
                    else {
                        updatePlayerHp(3);
                    }
                    circleActive = false;
                    timeSinceSpawn = 0;
                }
            } else {
                // No circle active — now we're truly done
                al_show_mouse_cursor(al_get_current_display());
                minigameBoxActive = false;
                enemyHit();
                BoxGameGroup->Clear();
            }
        } else {
            // Only spawn/process circle if still in allowed time
            if (!circleActive) {
                timeSinceSpawn += dt;
                if (timeSinceSpawn >= spawnCD) {
                    auto [randX, randY] = generateRandomPos();
                    circlePos = Engine::Point(randX, randY);
                    circleActive = true;
                    activeTime = 0.0f;
                    timeSinceSpawn = 0.0f;
                }
            } else {
                activeTime += dt;
                float dx = mouseX - circlePos.x;
                float dy = mouseY - circlePos.y;
                float dist = std::sqrt(dx * dx + dy * dy);

                if (dist <= circleRad) {
                    hitEffects.push_back({circlePos.x, circlePos.y, 0.3f, true});
                    circleActive = false;
                    timeSinceSpawn = 0;
                } else if (activeTime >= spawnCD) {
                    updatePlayerHp(3);
                    circleActive = false;
                    timeSinceSpawn = 0;
                }
            }
        }

        if (notesTime >= 10) {
            if (circleActive) {
                activeTime += dt;
                if (activeTime >= spawnCD) {
                    circleActive = false;
                    timeSinceSpawn = 0;
                }
            } else {
                al_show_mouse_cursor(al_get_current_display());
                minigameBoxActive = false;
                enemyHit();
                BoxGameGroup->Clear();
            }
        }
    }
    if (playerHitTimer > 0) {
        playerHitTimer -= dt;
    }
    if (enemyShakeTime > 0.0f) {
        enemyShakeTime -= dt;
        if (enemyShakeTime < 0.0f) enemyShakeTime = 0.0f;
    }
    frameTime += dt;
    if (frameTime >= frameInterval) {
        frameTime = 0.0f;
        currentEnemyFrame = (currentEnemyFrame + 1) % enemyFrames.size();
    }
}

void BattleScene::Draw() const {
    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;

    IScene::Draw();
    LabelGroup->Draw();

    if (!introAnimationPlaying) al_draw_rectangle(minigame_x1, minigame_y1, minigame_x2, minigame_y2, al_map_rgba(255,255,255,255), 7);
    if (minigameNotesActive) {
        NotesGroup->Draw();


        if (!introAnimationPlaying) {
            al_draw_rectangle(minigame_x1, minigame_y1, minigame_x2, minigame_y2, al_map_rgba(255,255,255,255), 7);
        }

        float centerX = minigame_x2 - (300 + notesGap) / 2.0f;
        float topY = minigame_y1 + 45;
        float midY = (minigame_y1 + minigame_y2) / 2.0f;
        float bottomY = minigame_y2 - 45;
        float radius = 37.0f;

        al_draw_circle(centerX, topY, radius, al_map_rgb(255, 0, 0),5);
        al_draw_circle(centerX, midY, radius, al_map_rgb(255, 255, 0),5);
        al_draw_circle(centerX, bottomY, radius, al_map_rgb(0, 255, 0),5);
        const ALLEGRO_FONT* font = Engine::Resources::GetInstance().GetFont("undertale.ttf", 32).get();
        ALLEGRO_COLOR white = al_map_rgb(255, 255, 255);
        al_draw_text(font, white, centerX, topY - 16, ALLEGRO_ALIGN_CENTER, "J");
        al_draw_text(font, white, centerX, midY - 16, ALLEGRO_ALIGN_CENTER, "K");
        al_draw_text(font, white, centerX, bottomY - 16, ALLEGRO_ALIGN_CENTER, "L");

        for (const auto& effect : hitEffects) {
            float alpha = effect.timeLeft / 0.3f;
            float radius = 64 + 64 * (1.0f - alpha); // pulse
            ALLEGRO_COLOR glow;

            if (effect.isBox) {
                glow = al_map_rgba_f(1.0, 1.0, 0.0, alpha); // yellow for box game
            } else {
                glow = al_map_rgba_f(1.0, 1.0, 1.0, alpha); // white for notes game
            }

            al_draw_circle(effect.x, effect.y, radius, glow, 4);

        }
    }
    if (negotiateActive) {
        std::cout << "negotiate" << std::endl;
        NegotiateGroup->Draw();
    }
    if (minigameBoxActive) {
        if (circleActive) {
            al_draw_circle(circlePos.x,circlePos.y,circleRad,al_map_rgba(255,255,0,255),4);
        }
        imageSprite->Draw();
    }
    if (introAnimationPlaying) {
        int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
        int h = Engine::GameEngine::GetInstance().GetScreenSize().y;

        // Draw white background circle first
        al_draw_filled_circle(w / 2, h / 2, introRadius, al_map_rgb(255, 255, 255));

        // Clip everything to the circle
        al_set_clipping_rectangle(w / 2 - introRadius, h / 2 - introRadius, introRadius * 2, introRadius * 2);

        // Draw enemy image scaled based on circle radius
            if (enemyFrames.size() >= 2 && enemyFrames[1]) { // Use 2nd sprite (index 1)
                ALLEGRO_BITMAP* introBmp = enemyFrames[1];
                int bmpW = al_get_bitmap_width(introBmp);
                int bmpH = al_get_bitmap_height(introBmp);
                float drawSize = introRadius * 1.33f;
                al_draw_scaled_bitmap(introBmp, 0, 0, bmpW, bmpH,
                                      w / 2 - drawSize / 2, h / 2 - drawSize / 2,
                                      drawSize, drawSize, 0);
        }

        // Reset clip
        al_reset_clipping_rectangle();
    }
    else
    {
        float enemyW = 300;
        float enemyH = 300;
        float enemyX = w / 2 - enemyW / 2;
        float enemyY = minigame_y1 - enemyH - 20;

        if (enemyShakeTime > 0.0f) {
            float shakeX = ((rand() % 100) / 100.0f - 0.5f) * 2 * enemyShakeMagnitude;
            float shakeY = ((rand() % 100) / 100.0f - 0.5f) * 2 * enemyShakeMagnitude;
            enemyX += shakeX;
            enemyY += shakeY;
        }

        float centerX = enemyX + enemyW / 2;
        float centerY = enemyY + enemyH / 2;
        float radius = std::max(enemyW, enemyH) / 2 + 10;
        al_draw_filled_circle(centerX, centerY, radius, al_map_rgb(255, 255, 255));

        if (!enemyFrames.empty()) {
            ALLEGRO_BITMAP* currentBmp = enemyFrames[currentEnemyFrame];
            al_draw_scaled_bitmap(currentBmp, 0, 0,
                al_get_bitmap_width(currentBmp),
                al_get_bitmap_height(currentBmp),
                enemyX, enemyY, enemyW, enemyH, 0);
        }
    }
    if (playerHitTimer > 0) {
        float alpha = playerHitTimer / 0.3f; // fade out
        ALLEGRO_COLOR redTint = al_map_rgba_f(1.0, 0.0, 0.0, alpha);
        al_draw_filled_rectangle(0, 0, w, h, redTint);
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
                    float centerX = minigame_x2 - (300 + notesGap) / 2.0f;
                    float y = (insideNote->lane  == 0) ? minigame_y1 + 45 :
                               (insideNote->lane  == 1) ? (minigame_y1 + minigame_y2) / 2.0f :
                                             minigame_y2 - 45;

                    hitEffects.push_back({centerX, y, 0.3f});
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
                    float centerX = minigame_x2 - (300 + notesGap) / 2.0f;
                    float y = (insideNote->lane  == 0) ? minigame_y1 + 45 :
                               (insideNote->lane  == 1) ? (minigame_y1 + minigame_y2) / 2.0f :
                                             minigame_y2 - 45;

                    hitEffects.push_back({centerX, y, 0.3f});

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
                    float centerX = minigame_x2 - (300 + notesGap) / 2.0f;
                    float y = (insideNote->lane == 0) ? minigame_y1 + 45 :
                               (insideNote->lane  == 1) ? (minigame_y1 + minigame_y2) / 2.0f :
                                             minigame_y2 - 45;

                    hitEffects.push_back({centerX, y, 0.3f});

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
            AudioHelper::StopSample(bgmInstance);
            bgmInstance = std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE>();
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
            AudioHelper::StopSample(bgmInstance);
            bgmInstance = std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE>();
            LabelGroup->Clear();
            NotesGroup->Clear();
            NegotiateGroup->Clear();
            BoxGameGroup->Clear();
            labelHpEnemy = nullptr;
            labelHpPlayer = nullptr;
            labelDesc = nullptr;
            labelEmo = nullptr;
            for (auto bmp : enemyFrames) {
                if (bmp) al_destroy_bitmap(bmp);
            }
            enemyFrames.clear();
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
    char buff[20];
    if (labelHpEnemy) {
        LabelGroup->RemoveObject(labelHpEnemy->GetObjectIterator());
    }

    sprintf(buff,"HP:%d/%d",enemyHp,enemyMaxHp);
    labelHpEnemy = new Engine::Label(buff, "undertale.ttf", 32, halfW - 250, halfH/2 + 125, 255, 255, 255, 255, 0.5, 0.5);
    LabelGroup->AddNewObject(labelHpEnemy);


    if (enemyHp <= 0 && enemyBitmap) {
        al_destroy_bitmap(enemyBitmap);
        enemyBitmap = nullptr;
        AudioHelper::StopSample(bgmInstance);
        bgmInstance = std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE>();
        LabelGroup->Clear();
        NotesGroup->Clear();
        NegotiateGroup->Clear();
        BoxGameGroup->Clear();
        labelHpEnemy = nullptr;
        labelHpPlayer = nullptr;
        labelDesc = nullptr;
        labelEmo = nullptr;
        for (auto bmp : enemyFrames) {
            if (bmp) al_destroy_bitmap(bmp);
        }
        enemyFrames.clear();
        GameData::getInstance().deadEnemies.insert(enemyName);
        Engine::GameEngine::GetInstance().ChangeScene("play");
    }

    idle = true;
    enemyShakeTime = enemyShakeDuration;
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
    playerHitTimer = 0.3f;
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
        AudioHelper::StopSample(bgmInstance);
        bgmInstance = std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE>();
        LabelGroup->Clear();
        NotesGroup->Clear();
        NegotiateGroup->Clear();
        BoxGameGroup->Clear();
        labelHpEnemy = nullptr;
        labelHpPlayer = nullptr;
        labelDesc = nullptr;
        labelEmo = nullptr;
        for (auto bmp : enemyFrames) {
            if (bmp) al_destroy_bitmap(bmp);
        }
        enemyFrames.clear();
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
    int minX = static_cast<int>(minigame_x1 + circleRad);
    int maxX = static_cast<int>(minigame_x2 - circleRad);
    int minY = static_cast<int>(minigame_y1 + circleRad);
    int maxY = static_cast<int>(minigame_y2 - circleRad);
    std::uniform_int_distribution<int> distSafeX(minX, maxX);
    std::uniform_int_distribution<int> distSafeY(minY, maxY);
    int x = distSafeX(rng);
    int y = distSafeY(rng);
    return {x, y};
}






