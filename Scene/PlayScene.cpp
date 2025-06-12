#include <algorithm>
#include <allegro5/allegro.h>
#include <cmath>
#include <fstream>
#include <functional>
#include <memory>
#include <queue>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <filesystem>
#include "Player/Player.hpp"
#include "NPC/Npc.hpp"
#include "Turret/Turret.hpp"
#include "Engine/AudioHelper.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Group.hpp"
#include "Engine/LOG.hpp"
#include "Engine/AnimatedSprite.hpp"
#include "Engine/Resources.hpp"
#include "PlayScene.hpp"
#include "Player/GameData.hpp"
#include "NPC/enemynpc.h"

#include <allegro5/allegro_primitives.h>


#include "BattleScene.hpp"
#include "UI/Component/Slider.hpp"
#include "UI/Component/Textbox.hpp"
#include "UI/Component/Label.hpp"
#include "UI/Component/ImageButton.hpp"
#include "NPC/water.h"

// TODO HACKATHON-4 (1/3): Trace how the game handles keyboard input.
// TODO HACKATHON-4 (2/3): Find the cheat code sequence in this file.
// TODO HACKATHON-4 (3/3): When the cheat code is entered, a plane should be spawned and added to the scene.
// TODO HACKATHON-5 (1/4): There's a bug in this file, which crashes the game when you win. Try to find it.
// TODO HACKATHON-5 (2/4): The "LIFE" label are not updated when you lose a life. Try to fix it.

bool PlayScene::DebugMode = false;
bool mapSwitched;
const std::vector<Engine::Point> PlayScene::directions = { Engine::Point(-1, 0), Engine::Point(0, -1), Engine::Point(1, 0), Engine::Point(0, 1) };
const int PlayScene::MapWidth = 25, PlayScene::MapHeight = 13;
const int PlayScene::BlockSize = 64;
const float PlayScene::DangerTime = 7.61;
const Engine::Point PlayScene::SpawnGridPoint = Engine::Point(-1, 0);
const Engine::Point PlayScene::EndGridPoint = Engine::Point(MapWidth, MapHeight - 1);


std::set<int> keyDown;
std::string nextDiag;
std::string nameDiag;
bool interactable;
float interactRange;
Npc* interactingNPC;
std::string enemyName;

Engine::Point PlayScene::GetClientSize() {
    return Engine::Point(MapWidth * BlockSize, MapHeight * BlockSize);
}


void PlayScene::Initialize() {
    interactable = false;
    interactRange = 64.0;
    interactingNPC = nullptr;
    enemyName = "";
    keyDown.clear();
    activeTextbox = nullptr;
    mapState.clear();
    keyStrokes.clear();
    ticks = 0;
    deathCountDown = -1;
    lives = 10;
    money = 150;
    SpeedMult = 1;
    std::cout << mapSwitched << std::endl;
    mapSwitched = false;
    // Add groups from bottom to top.
    AddNewObject(TileMapGroup = new Group());


    // Should support buttons.
    AddNewObject(NpcGroup = new Group());
    AddNewControlObject(UIGroup = new Group());
    ReadMap(Engine::Point(-1, -1));
    float playerhp = GameData::getInstance().playerHp;
    float playerposx = GameData::getInstance().playerPosX;
    float playerposy = GameData::getInstance().playerPosY;
    player = new Player(GameData::getInstance().playerSkin, playerposx, playerposy, playerhp, 200);
    AddNewObject(player);
    std::cout << playerhp << std::endl;

    // ReadEnemyWave();
    // mapDistance = CalculateBFSDistance();
    // ConstructUI();
    imgTarget = new Engine::Image("play/target.png", 0, 0);
    imgTarget->Visible = false;
    preview = nullptr;
    // UIGroup->AddNewObject(imgTarget);
    // Preload Lose Scene
    // Start BGM.
    if (MapId == 1) {
        // bgmInstance = AudioHelper::PlaySample("littleroot.mp3", true, AudioHelper::BGMVolume);
        std::string ed = GameData::getInstance().ending;
        if (ed == "good") {
            bgmInstance = AudioHelper::PlaySample("goodEnd.mp3",true,AudioHelper::BGMVolume);
        } else if (ed == "bad") {
            bgmInstance = AudioHelper::PlaySample("badEnd.mp3",true,AudioHelper::BGMVolume);
        } else if (ed == "normal") {
            bgmInstance = AudioHelper::PlaySample("normalEnd.mp3",true,AudioHelper::BGMVolume);
        } else {
            bgmInstance = AudioHelper::PlaySample("littleroot.mp3", true, AudioHelper::BGMVolume);
        }
    } else {
        if (GameData::getInstance().deadEnemies.size() == 4) {
            GameData::getInstance().ending = "bad";
        } else if (GameData::getInstance().friendlyEnemies.size() == 4) {
            GameData::getInstance().ending = "good";
        } else if (GameData::getInstance().friendlyEnemies.size() + GameData::getInstance().deadEnemies.size() == 4) {
            GameData::getInstance().ending = "normal";
        }
        bgmInstance = AudioHelper::PlaySample("dungeon.mp3", true, AudioHelper::BGMVolume); // or whatever your second map BGM is
    }
    if (bgmInstance) {
        al_attach_sample_instance_to_mixer(bgmInstance.get(), al_get_default_mixer());
    }
    PauseInitialize();

    //if groveChild is dead cannot groveDad will always be hostile
    if (GameData::getInstance().deadEnemies.count("groveChild")) {
       if (GameData::getInstance().friendlyEnemies.count("groveDad")) {
           GameData::getInstance().friendlyEnemies.erase("groveDad");
       }
    }


}

void PlayScene::PauseInitialize()
{
     const int mapPixelWidth = MapWidth * BlockSize;
    const int mapPixelHeight = MapHeight * BlockSize;

    const int pauseW = 670;
    const int pauseH = 720;
    const int pauseX = (mapPixelWidth - pauseW) / 2;
    const int pauseY = (mapPixelHeight - pauseH) / 2;
    pauseBackground = new Engine::Image("play/pause.png", pauseX, pauseY, pauseW, pauseH);
    pauseBackground->Visible = false;
    UIGroup->AddNewObject(pauseBackground);

    const int centerX = pauseX + pauseW / 2 + 10;
    int currentY = pauseY + 52;

    pauseLabel = new Engine::Label("Pause", "undertale.ttf", 64,
                                 centerX, currentY,
                                 0, 0, 0, 255, 0.5, 0.5);
    pauseLabel->Visible = false;
    UIGroup->AddNewObject(pauseLabel);
    currentY += 100;

    const int sliderOffsetX = pauseX + (pauseW - 490) / 2; // Center sliders
    const int sliderWidth = 490;
    const int sliderHeight = 4;

    BGMLabel = new Engine::Label("BGM Volume", "undertale.ttf", 32,
                               centerX, currentY,
                               0, 0, 0, 255, 0.5, 0.5);
    BGMLabel->Visible = false;
    UIGroup->AddNewObject(BGMLabel);
    currentY += 40;

    sliderBGM = new Slider(sliderOffsetX, currentY, sliderWidth, sliderHeight);
    sliderBGM->SetOnValueChangedCallback(std::bind(&PlayScene::OnBGMSlideValueChanged, this, std::placeholders::_1));
    sliderBGM->SetValue(AudioHelper::BGMVolume);
    sliderBGM->Visible = false;
    UIGroup->AddNewControlObject(sliderBGM);
    currentY += 40;

    SFXLabel = new Engine::Label("SFX Volume", "undertale.ttf", 32,
                               centerX, currentY,
                               0, 0, 0, 255, 0.5, 0.5);
    SFXLabel->Visible = false;
    UIGroup->AddNewObject(SFXLabel);
    currentY += 40;

    sliderSFX = new Slider(sliderOffsetX, currentY, sliderWidth, sliderHeight);
    sliderSFX->SetOnValueChangedCallback(std::bind(&PlayScene::OnSFXSlideValueChanged, this, std::placeholders::_1));
    sliderSFX->SetValue(AudioHelper::SFXVolume);
    sliderSFX->Visible = false;
    UIGroup->AddNewControlObject(sliderSFX);
    currentY += 80;

    const int buttonWidth = 370;
    const int buttonHeight = 60;
    const int buttonX = pauseX + (pauseW - buttonWidth) / 2;
    const int buttonSpacing = 20;

    continueBtn = new Engine::ImageButton("play/dirt.png", "play/floor.png", buttonX, currentY, buttonWidth, buttonHeight);
    continueBtn->SetOnClickCallback(std::bind(&PlayScene::ContinueOnClick, this));
    UIGroup->AddNewControlObject(continueBtn); // Changed from AddNewObject to AddNewControlObject
    continueBtn->Visible = false;
    continueLabel = new Engine::Label("Continue", "undertale.ttf", 48,
                                    buttonX + buttonWidth / 2, currentY + buttonHeight / 2,
                                    0, 0, 0, 255, 0.5, 0.5);
    continueLabel->Visible = false;
    UIGroup->AddNewObject(continueLabel);
    currentY += buttonHeight + buttonSpacing;

    retryBtn = new Engine::ImageButton("play/dirt.png", "play/floor.png", buttonX, currentY, buttonWidth, buttonHeight);
    retryBtn->SetOnClickCallback(std::bind(&PlayScene::RetryOnClick, this));
    UIGroup->AddNewControlObject(retryBtn);
    retryBtn->Visible = false;
    retryLabel = new Engine::Label("Retry", "undertale.ttf", 48,
                                buttonX + buttonWidth / 2, currentY + buttonHeight / 2,
                                0, 0, 0, 255, 0.5, 0.5);
    retryLabel->Visible = false;
    UIGroup->AddNewObject(retryLabel);
    currentY += buttonHeight + buttonSpacing;

    exitBtn = new Engine::ImageButton("play/dirt.png", "play/floor.png", buttonX, currentY, buttonWidth, buttonHeight);
    exitBtn->SetOnClickCallback(std::bind(&PlayScene::ExitOnClick, this));
    UIGroup->AddNewControlObject(exitBtn);
    exitBtn->Visible = false;
    exitLabel = new Engine::Label("Back", "undertale.ttf", 48,
                                buttonX + buttonWidth / 2, currentY + buttonHeight / 2,
                                0, 0, 0, 255, 0.5, 0.5);
    exitLabel->Visible = false;
    UIGroup->AddNewObject(exitLabel);
    currentY += buttonHeight + buttonSpacing;
}

void PlayScene::Terminate() {
    AudioHelper::StopSample(bgmInstance);
    IScene::Terminate();
}
void PlayScene::Update(float deltaTime) {
    // If we use deltaTime directly, then we might have Bullet-through-paper problem.
    // Reference: Bullet-Through-Paper
    UIGroup->Update(deltaTime);
    if (isPaused) {
        if (preview)
            preview->Update(deltaTime);
        return;
    }

    if (isTransitioning) {
        transitionAlpha += deltaTime;

        if (transitionAlpha >= 1.0f && !mapSwitched) {
            MapId = (MapId == 1) ? 2 : 1;
            ResetScene(pendingSpawnPoint);
            mapSwitched = true;
        }

        if (transitionAlpha >= 2.0f) {
            transitionAlpha = -1.0f;
            isTransitioning = false;
            mapSwitched = false;
        }

        return;
    }

    // Store the player's position at the beginning of the frame.
    Engine::Point originalPos = player->Position;

    // --- POST-UPDATE COLLISION AND BOUNDARY CHECKS ---
    if (player && !activeTextbox) {
        player->Update(deltaTime);
        if (CheckPlayerCollision(player->Position)) {
            player->Position = originalPos;
        }
        const float pushBack = 1.0f;
        for (auto& npc : NpcGroup->GetObjects()) {
            float dx = player->Position.x - npc->Position.x;
            float dy = player->Position.y - npc->Position.y;
            float distance = sqrt(dx * dx + dy * dy);
            float minDistance = BlockSize * 0.8f; // Same as collision check

            if (distance < minDistance) {
                float overlap = minDistance - distance;
                player->Position.x += (dx / distance) * overlap * pushBack;
                player->Position.y += (dy / distance) * overlap * pushBack;
            }
        }
        player->Position.x = std::max(BlockSize/2.f, std::min(player->Position.x, (float)(MapWidth * BlockSize - BlockSize/2)));
        player->Position.y = std::max(BlockSize/2.f, std::min(player->Position.y, (float)(MapHeight * BlockSize - BlockSize/2)));

    }
    // --- END OF COLLISION CHECKS ---

    // Transition check to other maps
    if (!isTransitioning && player) {
        const float transitionRange = BlockSize * 1.5f;
        const float bottomEdge = (MapHeight * BlockSize) - BlockSize;
        const float playerRightEdge = player->Position.x + (BlockSize / 2);
        const float playerLeftEdge = player->Position.x - (BlockSize / 2);

        if (MapId == 1 && playerRightEdge >= (MapWidth * BlockSize) - transitionRange &&
            player->Position.y >= bottomEdge + 10) {
            isTransitioning = true;
            transitionAlpha = 0.0f;
            pendingSpawnPoint = Engine::Point(BlockSize, player->Position.y);
            } else if (MapId == 2 && player->Position.y <= BlockSize &&
             std::abs(player->Position.x - (MapWidth * BlockSize) / 2) <= BlockSize) {
            isTransitioning = true;
            transitionAlpha = 0.0f;
            pendingSpawnPoint = Engine::Point((MapWidth * BlockSize) - BlockSize, player->Position.y);
        }
    }


    if (!NpcGroup->GetObjects().empty()) {
        for (auto& npc:NpcGroup->GetObjects()) {
            float npcPosX = npc->Position.x;
            float npcPosY = npc->Position.y;
            float playerPosX = player->Position.x;
            float playerPosY = player->Position.y;

            if (std::abs(playerPosX - npcPosX) < interactRange &&
                std::abs(playerPosY - npcPosY) < interactRange) {

                interactable = true;

                interactingNPC = dynamic_cast<Npc*>(npc);
                if (!interactingNPC)
                    interactingNPC = dynamic_cast<EnemyNpc*>(npc);
                // if (interactable) {std::cout << interactingNPC->getName() << std::endl;}
                nameDiag = interactingNPC->getName();
                break;

            } else {
                interactingNPC = nullptr;
                nameDiag = "";
                interactable = false;
            }

        }
    }

    if (isTransitioning) {
        transitionAlpha += deltaTime;
        if (transitionAlpha >= 1.0f && !mapSwitched) {
            if (MapId == 1) {
                MapId = 2;
            } else if (MapId == 2) {
                MapId = 1;
            }
            ResetScene(pendingSpawnPoint);
            mapSwitched = true;
        }

        // End transition after full cycle (2.0f)
        if (transitionAlpha >= 2.0f) {
            transitionAlpha = -1.0f;
            isTransitioning = false;
            mapSwitched = false;
        }
        return;
    }

    if (preview) {
        preview->Position = Engine::GameEngine::GetInstance().GetMousePosition();
        preview->Update(deltaTime);
    }
    NpcGroup->Update(deltaTime);
    TileMapGroup->Update(deltaTime);
}

void PlayScene::ResetScene(Engine::Point spawnPosition) {


    if (TileMapGroup) TileMapGroup->Clear();
    if (NpcGroup) NpcGroup->Clear();
    if (UIGroup) UIGroup->Clear();

    if (player) {
        RemoveObject(player->GetObjectIterator());
        player = nullptr;
    }

    if (spawnPosition.y >= (MapHeight * BlockSize - BlockSize)) {
        spawnPosition.y -= BlockSize;
    }
    GameData::getInstance().changeMap = true;
    AudioHelper::StopBGM(bgmId);
    Terminate();
    Initialize();

    PauseInitialize();
    ReadMap(spawnPosition);


}

bool PlayScene::CheckPlayerCollision(const Engine::Point& newPos) {
    const float playerRadius = BlockSize * 0.4f;
    const float npcRadius = BlockSize * 0.4f;
    const float collisionDistance = playerRadius + npcRadius;
    bool collision = false;

    for (auto& npc : NpcGroup->GetObjects()) {
        float dx = newPos.x - npc->Position.x;
        float dy = newPos.y - npc->Position.y;
        float distance = sqrt(dx * dx + dy * dy);

        if (distance < collisionDistance) {
            Engine::LOG(Engine::INFO) << "Collision detected with NPC at ("
                                     << npc->Position.x << ", " << npc->Position.y << ")";
            collision = true;
        }
    }

    // Check house tile collisions
    int gridX = static_cast<int>(newPos.x / BlockSize);
    int gridY = static_cast<int>(newPos.y / BlockSize);

    if (MapId == 1) {
        if (gridX >= 0 && gridX < MapWidth && gridY >= 0 && gridY < MapHeight) {
            if (mapState[gridY][gridX] == TILE_HOUSE ||
                mapState[gridY][gridX] == TILE_TREE) {
                collision = true;
                }
            else if (mapState[gridY][gridX] == TILE_WATER) {
                if (newPos.y < (gridY * BlockSize + BlockSize * 1.0f)) {
                    collision = true;
                }
            }
        }
    }

    // trunk‐only collisions:
    for (auto &b : treeColliders) {
        if (newPos.x >= b.x && newPos.x <= b.x + b.w &&
            newPos.y >= b.y && newPos.y <= b.y + b.h) {
            return true;
            }
    }
    return false;
}



void PlayScene::Draw() const {
    IScene::Draw();
    int screenW = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int screenH = Engine::GameEngine::GetInstance().GetScreenSize().y;
    if (DebugMode) {
        for (const auto& b : treeColliders) {
            al_draw_rectangle(b.x, b.y, b.x + b.w, b.y + b.h, al_map_rgb(255, 0, 0), 2);
        }
    }
    if (activeTextbox) {
        al_draw_filled_rectangle(0, 0, screenW, screenH, al_map_rgba(0, 0, 0, 160));
    }
    UIGroup->Draw();
    if (transitionAlpha >= 0) {
        float alpha = (transitionAlpha < 1.0f) ? transitionAlpha : (2.0f - transitionAlpha);
        al_draw_filled_rectangle(0, 0, screenW, screenH, al_map_rgba_f(0, 0, 0, alpha));
    }
    if (isPaused) {
        al_draw_filled_rectangle(
            0, 0,
            MapWidth * BlockSize, MapHeight * BlockSize,
            al_map_rgba(0, 0, 0, 128)
        );
        if (pauseBackground) pauseBackground->Draw();
        if (continueBtn) continueBtn->Draw();
        if (retryBtn) retryBtn->Draw();
        if (exitBtn) exitBtn->Draw();
        if (pauseLabel) pauseLabel->Draw();
        if (continueLabel) continueLabel->Draw();
        if (retryLabel) retryLabel->Draw();
        if (exitLabel) exitLabel->Draw();
        if (sliderBGM) sliderBGM->Draw();
        if (sliderSFX) sliderSFX->Draw();
        if (BGMLabel) BGMLabel->Draw();
        if (SFXLabel) SFXLabel->Draw();


    }
    if (GameData::getInstance().ending == "bad" && MapId == 1) {
        al_draw_filled_rectangle(0,0,screenW,screenH, al_map_rgba(0, 0, 0, 175));
    }
}
void PlayScene::OnMouseDown(int button, int mx, int my) {
    if (!Visible) return;
    if (isPaused) {
        if ((button & 1)) {
            if (sliderBGM->Visible) {
                sliderBGM->OnMouseDown(button, mx, my);
            }
            if (sliderSFX->Visible){
                sliderSFX->OnMouseDown(button, mx, my);
            }
            UIGroup->OnMouseDown(button, mx, my);
        }
        return;
    }
    // IScene::OnMouseDown(button, mx, my);
}
void PlayScene::OnMouseMove(int mx, int my) {
    if (!Visible) return;
    IScene::OnMouseMove(mx, my);
    if (isPaused) {
        if (sliderBGM->Visible) sliderBGM->OnMouseMove(mx, my);
        if (sliderSFX->Visible) sliderSFX->OnMouseMove(mx, my);
        UIGroup->OnMouseMove(mx, my);
        imgTarget->Visible = false;
        return;
    }
    UIGroup->OnMouseMove(mx, my);
    const int x = mx / BlockSize;
    const int y = my / BlockSize;
    imgTarget->Visible = true;
    imgTarget->Position.x = x * BlockSize;
    imgTarget->Position.y = y * BlockSize;
}
void PlayScene::OnMouseUp(int button, int mx, int my) {
    if (!Visible) return;
    IScene::OnMouseUp(button, mx, my);
    if (isPaused) {
        if (sliderBGM->Visible) sliderBGM->OnMouseUp(button, mx, my);
        if (sliderSFX->Visible) sliderSFX->OnMouseUp(button, mx, my);
        draggingBGM = false;
        draggingSFX = false;
    }
    if (!imgTarget->Visible)return;
    const int x = mx / BlockSize;
    const int y = my / BlockSize;
}
void PlayScene::OnKeyUp(int keyCode) {
    IScene::OnKeyUp(keyCode);
    keyDown.erase(keyCode);
    if (player) player->OnKeyUp(keyCode);
}
void PlayScene::OnKeyDown(int keyCode) {
    IScene::OnKeyDown(keyCode);
    keyDown.insert(keyCode);
    if (keyCode == ALLEGRO_KEY_ESCAPE && !activeTextbox)
    {
        isPaused = !isPaused;
        SetPauseUIVisible(isPaused);
        return;
    }

    if (isPaused) return;

    if (!activeTextbox) {
        keyDown.insert(keyCode);
        if (player) player->OnKeyDown(keyCode);
    }


    if (keyCode == ALLEGRO_KEY_TAB) {
        DebugMode = !DebugMode;
    }

    if (keyCode == ALLEGRO_KEY_B) {

    }


    // std::cout << interactable << std::endl;
    if (keyCode == ALLEGRO_KEY_E && interactable) {

        if (!activeTextbox) {
            RenderTextbox("intro");
        }
    }
    if (activeTextbox) {
        if (keyCode == ALLEGRO_KEY_ENTER || keyCode == ALLEGRO_KEY_SPACE) {
            if (!activeTextbox->isDone()) {
                std::cout << "skipping" << std::endl;
                activeTextbox->Skip();
            } else {
                std::cout << "not skipping" << std::endl;
                if (!nextDiag.empty()) {
                    UIGroup->RemoveObject(activeTextbox->GetObjectIterator());
                    if (dialogueNpcImage) {
                        UIGroup->RemoveObject(dialogueNpcImage->GetObjectIterator());
                        dialogueNpcImage = nullptr;
                    }
                    RenderTextbox(nextDiag);
                } else {

                    UIGroup->RemoveObject(activeTextbox->GetObjectIterator());
                    activeTextbox = nullptr;
                    if (dialogueNpcImage) {
                        UIGroup->RemoveObject(dialogueNpcImage->GetObjectIterator());
                        dialogueNpcImage = nullptr;
                    }
                    nameDiag = "";

                    if (interactingNPC->isHostile()) {
                        SaveState(interactingNPC->getName());
                        Engine::GameEngine::GetInstance().ChangeScene("battle");
                    }

                }
            }
        }
    }

    // if (keyCode == ALLEGRO_KEY_ESCAPE) {
    //     if (activeTextbox) {
    //         UIGroup->RemoveObject(activeTextbox->GetObjectIterator());
    //         activeTextbox = nullptr;
    //     }
    // }




    if (keyCode >= ALLEGRO_KEY_0 && keyCode <= ALLEGRO_KEY_9) {
        // Hotkey for Speed up.
        SpeedMult = keyCode - ALLEGRO_KEY_0;
    }
}

void PlayScene::SetPauseUIVisible(bool visible) {
    if (visible) {
        // Clear active previews
        if (preview && UIGroup) {
            UIGroup->RemoveObject(preview->GetObjectIterator());
            preview = nullptr;
        }
    }

    if (pauseBackground) pauseBackground->Visible = visible;
    if (continueBtn) continueBtn->Visible = visible;
    if (retryBtn) retryBtn->Visible = visible;
    if (exitBtn) exitBtn->Visible = visible;
    if (pauseLabel) pauseLabel->Visible = visible;
    if (continueLabel) continueLabel->Visible = visible;
    if (retryLabel) retryLabel->Visible = visible;
    if (exitLabel) exitLabel->Visible = visible;
    if (BGMLabel) BGMLabel->Visible = visible;
    if (SFXLabel) SFXLabel->Visible = visible;
    if (sliderBGM) sliderBGM->Visible = visible;
    if (sliderSFX) sliderSFX->Visible = visible;
}

void PlayScene::Hit() {
    lives--;
    if (lives <= 0) {
        Engine::GameEngine::GetInstance().ChangeScene("lose");
    }
}

void PlayScene::ReadMap(Engine::Point overrideSpawn) {
    treeColliders.clear();
    std::string filename = std::string("Resource/map") + std::to_string(MapId) + ".txt";
    // Read map file.
    char c;
    std::vector<int> mapData;
    std::ifstream fin(filename);
    while (fin >> c) {
        switch (c) {
            case '0': mapData.push_back(0); break;  // Dirt
            case '1': mapData.push_back(1); break;  // Floor
            case '2': mapData.push_back(2); break;  // NPC1
            case '3': mapData.push_back(3); break;  // NPC2
            case 'S': mapData.push_back(4); break;  // Player Spawn
            case 'T': mapData.push_back(5); break;  // Player spawned when change map
            case 'G': mapData.push_back(6); break;  // Enemy ghost
            case 'H': mapData.push_back(7); break;  // House Tile
            // case '8': mapData.push_back(8); break;  // Another dirt
            case 'W': mapData.push_back(9); break;  // Water
            case 'R': mapData.push_back(10); break; // Tree Tile
            case 'r': mapData.push_back(11); break; // Small Tree
            case '4': mapData.push_back(12); break; // Floor Dungeon 1
            case '5': mapData.push_back(13); break; // Floor Dungeon 2
            case '6': mapData.push_back(14); break; // Floor Dungeon 3
            case '7': mapData.push_back(15); break; // Floor Dungeon 4
            case '8': mapData.push_back(16); break; // Floor Dungeon 5
            case '9': mapData.push_back(17); break; // Floor Dungeon 6
            case 'a': mapData.push_back(18); break; // Floor Dungeon 7
            case 'b': mapData.push_back(19); break; // Floor Dungeon 8
            case 'c': mapData.push_back(20); break; // Floor Dungeon 9
            case 'd': mapData.push_back(21); break; // Floor Dungeon 10
            case 'e': mapData.push_back(22); break; // Floor Dungeon 11
            case 'f': mapData.push_back(23); break; // Floor Dungeon 12
            case 'g': mapData.push_back(24); break; // Floor Dungeon 13
            case 'h': mapData.push_back(25); break; // Floor Dungeon 14
            case 'i': mapData.push_back(26); break; // Floor Dungeon 15
            case 'j': mapData.push_back(27); break; // Floor Dungeon 16
            case 'k': mapData.push_back(28); break; // Floor Dungeon 17
            case 'l': mapData.push_back(29); break; // Floor Dungeon 18
            case 'm': mapData.push_back(30); break;
            case 'n': mapData.push_back(31); break; // start of floor map 1
            case 'o': mapData.push_back(32); break; // Floor Dungeon 21
            case 'p': mapData.push_back(33); break; // Floor Dungeon 22
            case 'q': mapData.push_back(34); break; // Floor Dungeon 23
            case 's': mapData.push_back(35); break; // Floor Dungeon 24
            case 't': mapData.push_back(36); break; // entry teleport
            case 'u': mapData.push_back(37); break; // Floor Dungeon 26
            case 'v': mapData.push_back(38); break; // Floor Dungeon 27
            case 'w': mapData.push_back(39); break;
            case 'x': mapData.push_back(40); break; // Floor Dungeon 29
            case 'y': mapData.push_back(41); break; // Floor Dungeon 30
            case 'z': mapData.push_back(42); break; // Floor Dungeon 31
            case 'A': mapData.push_back(43); break; // Floor Dungeon 32
            case 'B': mapData.push_back(44); break; // Floor Dungeon 33
            case 'C': mapData.push_back(45); break; // Floor Dungeon 34
            case 'D': mapData.push_back(46); break; // Floor Dungeon 35
            case 'E': mapData.push_back(47); break; // Floor Dungeon 36
            case 'F': mapData.push_back(48); break; // Floor Dungeon 37
            case 'I': mapData.push_back(49); break; // Floor Dungeon 38
            case 'J': mapData.push_back(50); break; // Floor Dungeon 39
            case 'K': mapData.push_back(51); break; // Floor Dungeon 40
            case 'L': mapData.push_back(52); break; // Floor Dungeon 41
            case 'M': mapData.push_back(53); break; // Floor Dungeon 42
            case 'N': mapData.push_back(54); break; // Floor Dungeon 43
            case 'O': mapData.push_back(55); break; // Floor Dungeon 44
            case 'P': mapData.push_back(56); break; // Floor Dungeon 45
            case 'Q': mapData.push_back(57); break; // Floor Dungeon 46
            case 'U': mapData.push_back(58); break; // Floor Dungeon 47
            case 'V': mapData.push_back(59); break; // Floor Dungeon 48
            case '!': mapData.push_back(60); break; // Floor Dungeon 49
            case '@': mapData.push_back(61); break; // Floor Dungeon 50
            case '#': mapData.push_back(62); break; // Floor Dungeon 51
            case '$': mapData.push_back(63); break; // Floor Dungeon 52
            case '%': mapData.push_back(64); break; // Floor Dungeon 53
            case '^': mapData.push_back(65); break; // Floor Dungeon 54
            case '&': mapData.push_back(66); break; // Floor Dungeon 55
            case '*': mapData.push_back(67); break; // Floor Dungeon 56
            case '(': mapData.push_back(68); break; // Floor Dungeon 57
            case ')': mapData.push_back(69); break; // Floor Dungeon 58
            case '-': mapData.push_back(70); break; // Floor Dungeon 59
            case '_': mapData.push_back(71); break; // Floor Dungeon 60
            case '+': mapData.push_back(72); break; // Floor Dungeon 61
            case '=': mapData.push_back(73); break; // Floor Dungeon 62
            case '{': mapData.push_back(74); break; // Floor Dungeon 63
            case '}': mapData.push_back(75); break; // Floor Dungeon 64
            case '[': mapData.push_back(76); break; // Floor Dungeon 65
            case ']': mapData.push_back(77); break; // Floor Dungeon 66
            case '\\': mapData.push_back(78); break; // Floor Dungeon 67
            case '|': mapData.push_back(79); break; // Floor Dungeon 68
            case ';': mapData.push_back(80); break; // Floor Dungeon 69
            case ':': mapData.push_back(81); break; // Floor Dungeon 70
            case '\'': mapData.push_back(82); break; // Floor Dungeon 71
            case '\"': mapData.push_back(83); break; // Floor Dungeon 72
            case ',': mapData.push_back(84); break; // Floor Dungeon 73
            case '<': mapData.push_back(85); break; // Floor Dungeon 74
            case '>': mapData.push_back(86); break; // Floor Dungeon 75
            case '.': mapData.push_back(87); break; // Floor Dungeon 76
            case '?': mapData.push_back(88); break; // Floor Dungeon 77
            case '/': mapData.push_back(89); break; // Floor Dungeon 78
            case '`': mapData.push_back(90); break; // Floor Dungeon 79
            case '~': mapData.push_back(91); break; // Floor Dungeon 80
            case 'X': mapData.push_back(297); break;
            case 'Y': mapData.push_back(298); break;
            case 'Z': mapData.push_back(299); break;
            case '\n':
            case '\r':
                if (static_cast<int>(mapData.size()) / MapWidth != 0)
                    throw std::ios_base::failure("Map data is corrupted.");
                break;
            default: throw std::ios_base::failure("Map data is corrupted.");
        }
    }
    fin.close();
    // Validate map data.
    if (static_cast<int>(mapData.size()) != MapWidth * MapHeight)
        throw std::ios_base::failure("Map data is corrupted.");
    // Store map in 2d array.
    mapState = std::vector<std::vector<TileType>>(MapHeight, std::vector<TileType>(MapWidth));
    Engine::Point spawnPoint;
    Engine::Point teleportPoint;

    std::vector<Engine::Point> bigTreeAnchors;
    std::vector<Engine::Point> smallTreeAnchors;
    for (int i = 0; i < MapHeight; i++) {
        for (int j = 0; j < MapWidth; j++) {
            int num = mapData[i*MapWidth + j];
            if (num == 10) { // 'R' = 128×128 tree
                // only top‐left of the 2×2 block
                bool top    = (i == 0 || mapData[(i-1)*MapWidth + j] != 10);
                bool left   = (j == 0 || mapData[i*MapWidth + j-1] != 10);
                if (top && left) bigTreeAnchors.emplace_back(j, i);
            }
            if (num == 11) { // 'r' = 64×128 tree
                // only the top row of each 2-high column
                bool top = (i == 0 || mapData[(i-1)*MapWidth + j] != 11);
                if (top) smallTreeAnchors.emplace_back(j, i);
            }
        }
    }

    // Find the top-left 'H' to anchor the single large image
    int houseAnchorX = -1, houseAnchorY = -1;
    for (int i = 0; i < MapHeight; i++) {
        for (int j = 0; j < MapWidth; j++) {
            if (mapData[i * MapWidth + j] == 7) { // 7 corresponds to 'H'
                houseAnchorY = i;
                houseAnchorX = j;
                goto found_anchor; // Exit loops once the first 'H' is found
            }
        }
    }
    found_anchor:;

    // Process the map tiles
    for (int i = 0; i < MapHeight; i++) {
        for (int j = 0; j < MapWidth; j++)
        {
            const int num = mapData[i * MapWidth + j];
            switch (num) {
                case 0:  // Dirt
                    mapState[i][j] = TILE_DIRT;
                    TileMapGroup->AddNewObject(new Engine::Image("play/cao.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 1:  // Floor
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("tiles/dirt.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 2: // NPC1
                {

                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("play/cao.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    Npc *npc1 = new Npc("play/enemy-10.png", j * BlockSize + BlockSize/2, i * BlockSize + BlockSize/2, "npc1");
                    if (MapId == 1 && (GameData::getInstance().ending == "good" || GameData::getInstance().ending == "")) {
                        NpcGroup->AddNewObject(npc1);
                    }

                    break;
                }
                case 3: // NPC2
                {
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("play/cao.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    Npc *npc2 = new Npc("play/enemy-9.png", j * BlockSize + BlockSize/2, i * BlockSize + BlockSize/2, "npc2");
                    if (MapId == 1 && (GameData::getInstance().ending == "good" || GameData::getInstance().ending == "")) {
                        NpcGroup->AddNewObject(npc2);
                    }
                    break;
                }
                case 4:  // Spawn point
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("pack/floor_purple.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    spawnPoint = Engine::Point(j * BlockSize + BlockSize/2, i * BlockSize + BlockSize/2);
                    break;

                case 5: //Teleport point map 2
                    if (MapId == 2) {
                        mapState[i][j] = TILE_FLOOR;
                        TileMapGroup->AddNewObject(new Engine::Image("pack/upper.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                        teleportPoint = Engine::Point(j * BlockSize + BlockSize/2, i * BlockSize + BlockSize/2);
                    }
                    break;
                case 6: // Ghost
                {
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("play/cao.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    std::string name = "enemy1";
                    if (!GameData::getInstance().deadEnemies.count(name)) {
                        Npc *npc3;
                        if (GameData::getInstance().friendlyEnemies.count(name)) {
                            npc3 = new EnemyNpc("enemy1","archivist_shade_", j * BlockSize + BlockSize / 2, i * BlockSize + BlockSize / 2, false);
                            std::cout << "friendly";
                            // NpcGroup->AddNewObject(npc3);
                        } else {
                            std::cout << "not friendly";
                            npc3 = new EnemyNpc("enemy1","archivist_shade_", j * BlockSize + BlockSize / 2, i * BlockSize + BlockSize / 2, true);
                            // NpcGroup->AddNewObject(npc3);
                        }
                        if (MapId == 1 && GameData::getInstance().ending == "good") {
                            NpcGroup->AddNewObject(npc3);
                        } else if (MapId == 2) {
                            NpcGroup->AddNewObject(npc3);
                        }

                    }

                    break;
                }
                case 7: // House tile
                {
                    mapState[i][j] = TILE_HOUSE;
                    // Draw a background tile underneath each part of the house
                    TileMapGroup->AddNewObject(new Engine::Image("play/cao.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                }
                case 8:
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("play/cao2.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 9: {
                    // mapState[i][j] = TILE_WATER;
                    //
                    // // Define the sequence of water animation frames.
                    // std::vector<std::string> waterFrames = {
                    //     "Resource/images/play/water1.png",
                    //     "Resource/images/play/water2.png",
                    //     "Resource/images/play/water3.png",
                    //     "Resource/images/play/water4.png",
                    //     "Resource/images/play/water5.png",
                    //     "Resource/images/play/water6.png",
                    //     "Resource/images/play/water7.png",
                    //     "Resource/images/play/water8.png"
                    // };
                    //
                    // // --- ADD THESE TWO LINES ---
                    // // Create the animated sprite and add it to the tile group.
                    // auto* waterTile = new Engine::AnimatedSprite(waterFrames, j * BlockSize, i * BlockSize, BlockSize, BlockSize, 0.0f, 0.0f, 1.2f);
                    // TileMapGroup->AddNewObject(waterTile);
                    // // -------------------------
                    // break;
                    mapState[i][j] = TILE_WATER;
                    TileMapGroup->AddNewObject(
                      new WaterTile("play/water", j * BlockSize, i * BlockSize, 1.0f, 0.2f)
                    );
                    Box waterBox;
                    waterBox.x = j * BlockSize - 16;
                    waterBox.y = i * BlockSize - 44;
                    waterBox.w = BlockSize + 32;
                    waterBox.h = BlockSize + 32;
                    treeColliders.push_back(waterBox);
                    break;
                }
                case 10:
                case 11: // 'r' = small tree
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(
                      new Engine::Image("play/cao.png", j*BlockSize, i*BlockSize, BlockSize, BlockSize)
                    );
                    break;
                case 12: // Floor Dungeon 1
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("pack/floor_purple.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 13: // Floor Dungeon 2
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("pack/upper_left.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 14: // Floor Dungeon 2
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("pack/upper.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 15: // Floor Dungeon 2
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("pack/upper_right.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 16: // Floor Dungeon 2
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("pack/right.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 17: // Floor Dungeon 2
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("pack/lower_right.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 18: // Floor Dungeon 2
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("pack/lower.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 19: // Floor Dungeon 2
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("pack/lower_left.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 20: // Floor Dungeon 2
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("pack/left.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 21: // Floor Dungeon 2
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("pack/middle.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 22: // Floor Dungeon 2
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("pack/one.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 23: // Floor Dungeon 2
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("pack/two.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 24: // Floor Dungeon 2
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("pack/three.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 25: // Floor Dungeon 2
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("pack/four.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 26: // Floor Dungeon 2
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("pack/stained1.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 27: // Floor Dungeon 2
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("pack/stained2.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 28: // Floor Dungeon 2
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("pack/stained3.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 29: // Floor Dungeon 2
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("pack/stained4.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 30: // Floor Dungeon 2
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("pack/middle.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    // Now add the sword on top of this tile
                    TileMapGroup->AddNewObject(new Engine::Image("pack/big_pouch.png",
                        j * BlockSize + (BlockSize - 64) / 2,  // center horizontally (assuming sword is 32px wide)
                        i * BlockSize + (BlockSize - 64) / 2,  // center vertically (assuming sword is 32px tall)
                        64, 64)); // size of your sword image
                    break;
                case 31: // 'n'
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("pack/door1.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 32: // 'o'
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("pack/door2.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 33: // 'p'
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("tiles/horizontal_mid.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 34: // 'q'
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("tiles/horizontal_right.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 35: // 's'
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("tiles/vertical_down.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 36: // 't'
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("tiles/vertical_mid.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));

                    break;
                case 37: // 'u'
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("tiles/vertical_up.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 38: // 'v'
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("tiles/water/left.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 39: // 'w'
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("tiles/water/middle.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 40: // x
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("pack/floor_purple.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    // Now add the sword on top of this tile
                    TileMapGroup->AddNewObject(new Engine::Image("pack/sword_left.png",
                        j * BlockSize + (BlockSize - 64) / 2,  // center horizontally (assuming sword is 32px wide)
                        i * BlockSize + (BlockSize - 64) / 2,  // center vertically (assuming sword is 32px tall)
                        64, 64)); // size of your sword image
                    break;
                case 41: // y
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("pack/floor_purple.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    // Now add the sword on top of this tile
                    TileMapGroup->AddNewObject(new Engine::Image("pack/sword_right.png",
                        j * BlockSize + (BlockSize - 64) / 2,  // center horizontally (assuming sword is 32px wide)
                        i * BlockSize + (BlockSize - 64) / 2,  // center vertically (assuming sword is 32px tall)
                        64, 64)); // size of your sword image
                    break;
                case 42: // z
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("pack/floor_purple.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    // Now add the sword on top of this tile
                    TileMapGroup->AddNewObject(new Engine::Image("pack/candles2.png",
                        j * BlockSize + (BlockSize - 64) / 2,  // center horizontally (assuming sword is 32px wide)
                        i * BlockSize + (BlockSize - 64) / 2,  // center vertically (assuming sword is 32px tall)
                        64, 64)); // size of your sword image
                    break;
                case 43: // Floor Dungeon 2
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("pack/walls/1.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 44: // Floor Dungeon 2
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("pack/walls/2.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 45: // Floor Dungeon 2
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("pack/walls/3.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 46: // Floor Dungeon 2
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("pack/walls/4.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 47: // Floor Dungeon 2
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("pack/walls/5.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 48: // Floor Dungeon 2
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("pack/walls/6.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 49: // Floor Dungeon 2
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("pack/walls/7.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 50: // Floor Dungeon 2
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("pack/walls/8.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 51: // Floor Dungeon 2
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("pack/walls/9.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 52: // Floor Dungeon 2
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("pack/walls/10.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 53: // Floor Dungeon 2
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("pack/walls/11.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 54: // Floor Dungeon 2
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("pack/walls/12.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 55: // Floor Dungeon 2
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("pack/walls/13.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 56: // Floor Dungeon 2
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("pack/walls/14.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 57: // Floor Dungeon 2
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("pack/walls/15.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 59: // Floor Dungeon 2
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("pack/walls/16.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 60: // !
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("pack/floor_purple.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    // Now add the sword on top of this tile
                    TileMapGroup->AddNewObject(new Engine::Image("pack/pouches_left.png",
                        j * BlockSize + (BlockSize - 64) / 2,  // center horizontally (assuming sword is 32px wide)
                        i * BlockSize + (BlockSize - 64) / 2,  // center vertically (assuming sword is 32px tall)
                        64, 64)); // size of your sword image
                    break;
                case 61: // @
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("pack/floor_purple.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    // Now add the sword on top of this tile
                    TileMapGroup->AddNewObject(new Engine::Image("pack/pouches_right.png",
                        j * BlockSize + (BlockSize - 64) / 2,  // center horizontally (assuming sword is 32px wide)
                        i * BlockSize + (BlockSize - 64) / 2,  // center vertically (assuming sword is 32px tall)
                        64, 64)); // size of your sword image
                    break;
                case 62: // #
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("pack/floor_purple.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    // Now add the sword on top of this tile
                    TileMapGroup->AddNewObject(new Engine::Image("pack/big_pouch.png",
                        j * BlockSize + (BlockSize - 64) / 2,  // center horizontally (assuming sword is 32px wide)
                        i * BlockSize + (BlockSize - 64) / 2,  // center vertically (assuming sword is 32px tall)
                        64, 64)); // size of your sword image
                    break;
                case 63: // $
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("pack/floor_purple.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    // Now add the sword on top of this tile
                    TileMapGroup->AddNewObject(new Engine::Image("pack/candles.png",
                        j * BlockSize + (BlockSize - 64) / 2,  // center horizontally (assuming sword is 32px wide)
                        i * BlockSize + (BlockSize - 64) / 2,  // center vertically (assuming sword is 32px tall)
                        64, 64)); // size of your sword image
                    break;
                case 64: // '%'
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("tiles/water/right.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 65: // Floor Dungeon 2
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("tiles/corner1.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 66: // Floor Dungeon 2
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("tiles/corner2.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 67: // Floor Dungeon 2
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("tiles/corner3.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 68: // Floor Dungeon 2
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("tiles/corner4.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 69:
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("tiles/head_grass.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 70:
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("tiles/horizontal_left.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 71:
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("play/cao.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    TileMapGroup->AddNewObject(new Engine::Image("play/sign1.png",
                        j * BlockSize + (BlockSize - 64) / 2,  // center horizontally (assuming sword is 32px wide)
                        i * BlockSize + (BlockSize - 64) / 2,  // center vertically (assuming sword is 32px tall)
                        64, 64)); // size of your sword image
                    break;
                case 72:
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("play/cao.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    TileMapGroup->AddNewObject(new Engine::Image("play/bush1.png",
                        j * BlockSize + (BlockSize - 64) / 2,  // center horizontally (assuming sword is 32px wide)
                        i * BlockSize + (BlockSize - 64) / 2,  // center vertically (assuming sword is 32px tall)
                        64, 64)); // size of your sword image
                    break;
                case 73:
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("play/cao.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    TileMapGroup->AddNewObject(new Engine::Image("play/bush2.png",
                        j * BlockSize + (BlockSize - 64) / 2,  // center horizontally (assuming sword is 32px wide)
                        i * BlockSize + (BlockSize - 64) / 2,  // center vertically (assuming sword is 32px tall)
                        64, 64)); // size of your sword image
                    break;
                case 74:
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("play/cao.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    TileMapGroup->AddNewObject(new Engine::Image("play/bush3.png",
                        j * BlockSize + (BlockSize - 64) / 2,  // center horizontally (assuming sword is 32px wide)
                        i * BlockSize + (BlockSize - 64) / 2,  // center vertically (assuming sword is 32px tall)
                        64, 64)); // size of your sword image
                    break;
                case 75: // }
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("play/dirt.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    TileMapGroup->AddNewObject(new Engine::Image("play/chair.png",
                        j * BlockSize + (BlockSize - 64) / 2,  // center horizontally (assuming sword is 32px wide)
                        i * BlockSize + (BlockSize - 64) / 2,  // center vertically (assuming sword is 32px tall)
                        64, 64)); // size of your sword image
                    break;
                case 76: // [
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("tiles/dirt.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    TileMapGroup->AddNewObject(new Engine::Image("play/tree_log.png",
                        j * BlockSize + (BlockSize - 64) / 2,  // center horizontally (assuming sword is 32px wide)
                        i * BlockSize + (BlockSize - 64) / 2,  // center vertically (assuming sword is 32px tall)
                        64, 64)); // size of your sword image
                    break;
                case 77:
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("play/cao.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    TileMapGroup->AddNewObject(new Engine::Image("play/chest.png",
                        j * BlockSize + (BlockSize - 64) / 2,  // center horizontally (assuming sword is 32px wide)
                        i * BlockSize + (BlockSize - 64) / 2,  // center vertically (assuming sword is 32px tall)
                        64, 64)); // size of your sword image
                    break;
                case 78:
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("tiles/upper_left_corner.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 79:
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("tiles/upper.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 80:
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("tiles/upper_right_corner.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 81:
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("tiles/left.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 82:
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("tiles/right.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 83:
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("tiles/bottom_left_corner.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 84:
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("tiles/bottom.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 85:
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("tiles/bottom_right_corner.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 86:
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("play/cao.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    TileMapGroup->AddNewObject(new Engine::Image("play/fence.png",
                        j * BlockSize + (BlockSize - 64) / 2,  // center horizontally (assuming sword is 32px wide)
                        i * BlockSize + (BlockSize - 64) / 2,  // center vertically (assuming sword is 32px tall)
                        64, 64)); // size of your sword image
                    break;
                case 87:
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("tiles/soft1.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 88:
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("tiles/soft2.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 89:
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("tiles/soft3.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 90:
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("tiles/soft4.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 91:
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("play/dirt.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    TileMapGroup->AddNewObject(new Engine::Image("play/tree1.png",
                        j * BlockSize + (BlockSize - 64) / 2,  // center horizontally (assuming sword is 32px wide)
                        i * BlockSize + (BlockSize - 64) / 2,  // center vertically (assuming sword is 32px tall)
                        64, 64)); // size of your sword image
                    break;

                case 297: // V groveDad
                {
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("pack/middle.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    std::string name = "groveDad";
                    if (!GameData::getInstance().deadEnemies.count(name)) {
                        Npc* npc3;
                        if (GameData::getInstance().friendlyEnemies.count(name)) {
                            npc3 = new EnemyNpc(name,"grove_warden_", j * BlockSize + BlockSize/2, i * BlockSize + BlockSize/2, false);
                            // NpcGroup->AddNewObject(npc3);

                        } else {
                            npc3 = new EnemyNpc(name,"grove_warden_", j * BlockSize + BlockSize/2, i * BlockSize + BlockSize/2,true);
                            // NpcGroup->AddNewObject(npc3);
                        }
                        if (MapId == 1 && GameData::getInstance().ending == "good") {
                            NpcGroup->AddNewObject(npc3);
                        } else if (MapId == 2) {
                            NpcGroup->AddNewObject(npc3);
                        }
                    }

                    break;
                }
                case 298: {
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("pack/middle.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    std::string name = "groveChild";
                    if (!GameData::getInstance().deadEnemies.count(name)) {
                        Npc* npc3;
                        if (GameData::getInstance().friendlyEnemies.count(name)) {
                            npc3 = new EnemyNpc(name,"grove_child_", j * BlockSize + BlockSize/2, i * BlockSize + BlockSize/2, false);
                            std::cout << "friendly";
                            // NpcGroup->AddNewObject(npc3);
                        } else {
                            std::cout << "not friendly";
                            npc3 = new EnemyNpc(name,"grove_child_", j * BlockSize + BlockSize/2, i * BlockSize + BlockSize/2,true);
                            // NpcGroup->AddNewObject(npc3);
                        }
                        if (MapId == 1 && GameData::getInstance().ending == "good") {
                            NpcGroup->AddNewObject(npc3);
                        } else if (MapId == 2) {
                            NpcGroup->AddNewObject(npc3);
                        }
                    }

                    break;
                }
                case 299:
                {
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("play/cao.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    std::string name = "jesterKnight";
                    if (!GameData::getInstance().deadEnemies.count(name)) {
                        Npc *npc3;
                        if (GameData::getInstance().friendlyEnemies.count(name)) {
                            npc3 = new EnemyNpc(name, "jester_knight_", j * BlockSize + BlockSize/2, i * BlockSize + BlockSize/2, false);
                            std::cout << "friendly";
                            // NpcGroup->AddNewObject(npc3);
                        } else {
                            std::cout << "not friendly";
                            npc3 = new EnemyNpc(name, "jester_knight_", j * BlockSize + BlockSize/2, i * BlockSize + BlockSize/2,true);
                            // NpcGroup->AddNewObject(npc3);
                        }
                        if (MapId == 1 && GameData::getInstance().ending == "good") {
                            NpcGroup->AddNewObject(npc3);
                        } else if (MapId == 2) {
                            NpcGroup->AddNewObject(npc3);
                        }
                    }

                        break;
                    }
                }

        }
    }

    // Now, draw the single large house image over the top
    if (houseAnchorX != -1) {
        const int houseWidthInTiles = 3;
        const int houseHeightInTiles = 4;
        TileMapGroup->AddNewObject(new Engine::Image("play/house.png",
            houseAnchorX * BlockSize,
            houseAnchorY * BlockSize,
            houseWidthInTiles * BlockSize,
            houseHeightInTiles * BlockSize));

        // // Draw tree to the right of the house (96x128 = 1.5x2 tiles)
        // TileMapGroup->AddNewObject(new Engine::Image("play/tree1.png",
        //     (houseAnchorX + houseWidthInTiles) * BlockSize,  // Right of house
        //     houseAnchorY * BlockSize,                        // Same Y as house top
        //     96, 128));                                      // Exact pixel dimensions
    Box houseBox;
    houseBox.w = 206;  // adjust as needed
    houseBox.h = 270;   // only the bottom row blocks movement
    houseBox.x = houseAnchorX * BlockSize + (3 * BlockSize - houseBox.w) / 2.0f ;
    houseBox.y = (houseAnchorY + 3) * BlockSize - 196;  // bottom row (row 3 of 4)
    treeColliders.push_back(houseBox);
    }



    for (auto &p : bigTreeAnchors) {
    TileMapGroup->AddNewObject(new Engine::Image(
        "play/tree1.png",
        p.x * BlockSize - 32,      // center 128px over 2 tiles
        p.y * BlockSize - 32,
        128, 128
    ));
}

    // for (auto &p : smallTreeAnchors) {
    //     Box b;
    //     b.w = 24;
    //     b.h = 40;
    //     b.x = p.x * BlockSize + (64 - b.w)/2.0f;
    //     b.y = (p.y * BlockSize - 64) + (128 - b.h); // Corrected to match actual draw position
    //     treeColliders.push_back(b);
    // }

    for (auto &p : smallTreeAnchors) {
        TileMapGroup->AddNewObject(new Engine::Image(
            "play/tree2.png",
            p.x * BlockSize,           // left aligned
            p.y * BlockSize - 64 + 106,      // 128px high → raise 1 tile
            64, 128
        ));
    }



    // big 128×128 trees:
    for (auto &p : bigTreeAnchors) {
        // Sprite is 128×128, bottom‐half is trunk, ~32px wide at center:
        Box b;
        b.w = 112;
        b.h = 128;
        b.x = p.x*BlockSize + (128 - b.w)/2.0f - 32;   // center it horizontally over the 2 tiles
        b.y = p.y * BlockSize + (128 - b.h) - 40;        // bottom half of sprite
        treeColliders.push_back(b);
    }

    // small 64×128 trees:
    for (auto &p : smallTreeAnchors) {
        // Sprite is  64×128, trunk is maybe 24px wide, 40px tall at the bottom:
        Box b;
        b.w = 24 + 24;
        b.h = 102;
        b.x = p.x*BlockSize + (64 - b.w)/2.0f;    // center over this single tile
        b.y = p.y*BlockSize + (128 - b.h);        // bottom of sprite
        treeColliders.push_back(b);
    }

    // Resume original spawning logic
    if (!GameData::getInstance().spawned) {
        GameData::getInstance().spawned = true;
        GameData::getInstance().playerPosX = spawnPoint.x;
        GameData::getInstance().playerPosY = spawnPoint.y;
    }
    if (MapId == 2) {
        const int block = BlockSize;
        const int mapPixelW = MapWidth * block;
        const int mapPixelH = MapHeight * block;

        const int innerX = 1 * block;
        const int innerY = 1 * block;
        const int innerW = 23 * block;
        const int innerH = 11 * block;

        // Top wall
        treeColliders.push_back(Box{
            0, 15,
            mapPixelW / 2 - 20, innerY
        });

        treeColliders.push_back(Box{
            mapPixelW / 2 + 20, 15,
            mapPixelW / 2, innerY
        });



        // Bottom wall
        treeColliders.push_back(Box{
            0, innerY + innerH,
            mapPixelW, mapPixelH - (innerY + innerH)
        });

        // Left wall
        treeColliders.push_back(Box{
            0 - 10, innerY,
            innerX, innerH
        });

        // Right wall
        treeColliders.push_back(Box{
            innerX + innerW + 10, innerY,
            mapPixelW - (innerX + innerW), innerH
        });
    }

    if (GameData::getInstance().spawned && GameData::getInstance().changeMap) {
        GameData::getInstance().playerPosX = teleportPoint.x;
        GameData::getInstance().playerPosY = teleportPoint.y;
        GameData::getInstance().changeMap = false;
    }
    if (overrideSpawn.x != -1 && overrideSpawn.y != -1) spawnPoint = overrideSpawn;
    // int halfW = Engine::GameEngine::GetInstance().GetScreenSize().x / 2;
    // int halfH = Engine::GameEngine::GetInstance().GetScreenSize().y / 2;
    // player = new Player("play/enemy-11.png",
    //                    spawnPoint.x ? spawnPoint.x : halfW,
    //                    spawnPoint.y ? spawnPoint.y : halfH,
    //                    10, 200);
    // AddNewObject(player);
}

void PlayScene::ConstructUI() {
}

void PlayScene::UIBtnClicked(int id) {
    if (preview){
        UIGroup->RemoveObject(preview->GetObjectIterator());
        preview = nullptr;
    }
    if (!preview) return;
    OnMouseMove(Engine::GameEngine::GetInstance().GetMousePosition().x, Engine::GameEngine::GetInstance().GetMousePosition().y);
}



Engine::Point PlayScene::getScreenSize() {
    return Engine::GameEngine::GetInstance().GetScreenSize();
}

void PlayScene::RenderTextbox(std::string key) {
    keyDown.clear();
    Engine::Point ScreenSize = getScreenSize();
    std::ifstream jsonFileStream("Resource/jsonData/dialogue.json");
    // std::cout << std::filesystem::exists("Resource/jsonData/enemyInfo.json") << std::endl;
    nlohmann::json data = nlohmann::json::parse(jsonFileStream);
    std::string text;
    if (interactingNPC->isHostile()) {
        text = data["dialogue"][nameDiag]["hostile"][key]["text"];
        nextDiag = data["dialogue"][nameDiag]["hostile"][key]["next"];
    } else {
        text = data["dialogue"][nameDiag]["friendly"][key]["text"];
        nextDiag = data["dialogue"][nameDiag]["friendly"][key]["next"];
    }

    if (MapId == 1 && GameData::getInstance().ending == "good") {
        text = "Congratulations!";
    }


    // std::cout << std::filesystem::current_path() << std::endl;


    activeTextbox = new Engine::Textbox(text,ScreenSize);
    if (dialogueNpcImage) {
        UIGroup->RemoveObject(dialogueNpcImage->GetObjectIterator());
        dialogueNpcImage = nullptr;
    }
    std::string portraitPath = "play/portrait_" + nameDiag + ".png";
    int textboxHeight = 400;
    int portraitHeight = 300;
    int portraitY = ScreenSize.y - textboxHeight - portraitHeight / 2 - 100;
    dialogueNpcImage = new Engine::Image(portraitPath, 100, portraitY, 300, 300);
    UIGroup->AddNewObject(dialogueNpcImage);
    UIGroup->AddNewObject(activeTextbox);
    jsonFileStream.close();
}

void PlayScene::SaveState(std::string enemyName) {
    GameData::getInstance().playerHp = player->getHp();
    GameData::getInstance().playerPosX = player->Position.x;
    GameData::getInstance().playerPosY = player->Position.y;
    GameData::getInstance().currentEnemy = interactingNPC;
}

void PlayScene::OnBGMSlideValueChanged(float value) {
    if (bgmInstance) {
        al_set_sample_instance_gain(bgmInstance.get(), value);
    }
    AudioHelper::BGMVolume = value;
}

void PlayScene::OnSFXSlideValueChanged(float value) {
    AudioHelper::SFXVolume = value;
}
void PlayScene::ContinueOnClick() {
    isPaused = false;
    SetPauseUIVisible(false);
}

void PlayScene::RetryOnClick() {
    GameData::getInstance().spawned = false;
    GameData::getInstance().changeMap = false;
    isPaused = false;
    SetPauseUIVisible(false);
    Engine::GameEngine::GetInstance().ChangeScene("play");
}

void PlayScene::ExitOnClick() {
    GameData::getInstance().spawned = false;
    GameData::getInstance().changeMap = false;
    isPaused = false;
    SetPauseUIVisible(false);
    std::cout << GameData::getInstance().Username << std::endl;
    //write to safe file
    std::string path = (std::filesystem::path(PROJECT_SOURCE_DIR) / "Resource/jsonData/SaveFile.json").string();
    std::ifstream jsonFileStream(path);
    nlohmann::json jsonData = nlohmann::json::parse(jsonFileStream);
    jsonFileStream.close();
    jsonData[GameData::getInstance().Username]["DeadEnemy"] = GameData::getInstance().deadEnemies;
    jsonData[GameData::getInstance().Username]["FriendlyEnemy"] = GameData::getInstance().friendlyEnemies;

    std::ofstream jsonOut(path);
    jsonOut << jsonData.dump(4);
    jsonOut.close();



    Engine::GameEngine::GetInstance().ChangeScene("stage-select");
}
