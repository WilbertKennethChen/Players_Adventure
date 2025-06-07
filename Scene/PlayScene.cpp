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
#include "Engine/Resources.hpp"
#include "PlayScene.hpp"
#include "Player/GameData.hpp"

#include <allegro5/allegro_primitives.h>


#include "BattleScene.hpp"
#include "UI/Component/Slider.hpp"
#include "UI/Component/Textbox.hpp"
#include "UI/Component/Label.hpp"
#include "UI/Component/ImageButton.hpp"

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
const std::vector<int> PlayScene::code = {
    ALLEGRO_KEY_UP, ALLEGRO_KEY_UP, ALLEGRO_KEY_DOWN, ALLEGRO_KEY_DOWN,
    ALLEGRO_KEY_LEFT, ALLEGRO_KEY_RIGHT, ALLEGRO_KEY_LEFT, ALLEGRO_KEY_RIGHT,
    ALLEGRO_KEY_B, ALLEGRO_KEY_A, ALLEGRO_KEYMOD_SHIFT, ALLEGRO_KEY_ENTER
};

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
    // AddNewObject(GroundEffectGroup = new Group());
    // AddNewObject(TowerGroup = new Group());
    // AddNewObject(EnemyGroup = new Group());
    // AddNewObject(BulletGroup = new Group());
    // AddNewObject(EffectGroup = new Group());
    // Should support buttons.
    AddNewObject(NpcGroup = new Group());
    AddNewControlObject(UIGroup = new Group());
    ReadMap(Engine::Point(-1, -1));
    float playerhp = GameData::getInstance().playerHp;
    float playerposx = GameData::getInstance().playerPosX;
    float playerposy = GameData::getInstance().playerPosY;
    player = new Player("play/panda.png",playerposx, playerposy,playerhp,200,32,32,4);
    AddNewObject(player);
    std::cout << playerhp << std::endl;


    AddNewControlObject(UIGroup = new Group());

    // ReadEnemyWave();
    // mapDistance = CalculateBFSDistance();
    // ConstructUI();
    imgTarget = new Engine::Image("play/target.png", 0, 0);
    imgTarget->Visible = false;
    preview = nullptr;
    // UIGroup->AddNewObject(imgTarget);
    // Preload Lose Scene
    deathBGMInstance = Engine::Resources::GetInstance().GetSampleInstance("astronomia.ogg");
    if (deathBGMInstance) {
        al_attach_sample_instance_to_mixer(deathBGMInstance.get(), al_get_default_mixer());
    }
    // Start BGM.
    bgmInstance = AudioHelper::PlaySample("littleroot.mp3", true, AudioHelper::BGMVolume);
    if (bgmInstance) {
        al_attach_sample_instance_to_mixer(bgmInstance.get(), al_get_default_mixer());
    }
    PauseInitialize();
}

void PlayScene::PauseInitialize()
{
     const int mapPixelWidth = MapWidth * BlockSize;
    const int mapPixelHeight = MapHeight * BlockSize;

    const int pauseW = 640;
    const int pauseH = 650;
    const int pauseX = (mapPixelWidth - pauseW) / 2;
    const int pauseY = (mapPixelHeight - pauseH) / 2;
    pauseBackground = new Engine::Image("play/sand.png", pauseX, pauseY, pauseW, pauseH);
    pauseBackground->Visible = false;
    UIGroup->AddNewObject(pauseBackground);

    const int centerX = pauseX + pauseW / 2;
    int currentY = pauseY + 40;

    pauseLabel = new Engine::Label("Pause", "pirulen.ttf", 64,
                                 centerX, currentY,
                                 0, 0, 0, 255, 0.5, 0.5);
    pauseLabel->Visible = false;
    UIGroup->AddNewObject(pauseLabel);
    currentY += 100;

    const int sliderOffsetX = pauseX + (pauseW - 190) / 2; // Center sliders
    const int sliderWidth = 190;
    const int sliderHeight = 4;

    BGMLabel = new Engine::Label("BGM Volume", "pirulen.ttf", 24,
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

    SFXLabel = new Engine::Label("SFX Volume", "pirulen.ttf", 24,
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
    continueLabel = new Engine::Label("Continue", "pirulen.ttf", 48,
                                    buttonX + buttonWidth / 2, currentY + buttonHeight / 2,
                                    0, 0, 0, 255, 0.5, 0.5);
    continueLabel->Visible = false;
    UIGroup->AddNewObject(continueLabel);
    currentY += buttonHeight + buttonSpacing;

    retryBtn = new Engine::ImageButton("play/dirt.png", "play/floor.png", buttonX, currentY, buttonWidth, buttonHeight);
    retryBtn->SetOnClickCallback(std::bind(&PlayScene::RetryOnClick, this));
    UIGroup->AddNewControlObject(retryBtn);
    retryBtn->Visible = false;
    retryLabel = new Engine::Label("Retry", "pirulen.ttf", 48,
                                buttonX + buttonWidth / 2, currentY + buttonHeight / 2,
                                0, 0, 0, 255, 0.5, 0.5);
    retryLabel->Visible = false;
    UIGroup->AddNewObject(retryLabel);
    currentY += buttonHeight + buttonSpacing;

    exitBtn = new Engine::ImageButton("play/dirt.png", "play/floor.png", buttonX, currentY, buttonWidth, buttonHeight);
    exitBtn->SetOnClickCallback(std::bind(&PlayScene::ExitOnClick, this));
    UIGroup->AddNewControlObject(exitBtn);
    exitBtn->Visible = false;
    exitLabel = new Engine::Label("Back", "pirulen.ttf", 48,
                                buttonX + buttonWidth / 2, currentY + buttonHeight / 2,
                                0, 0, 0, 255, 0.5, 0.5);
    exitLabel->Visible = false;
    UIGroup->AddNewObject(exitLabel);
    currentY += buttonHeight + buttonSpacing;
}

void PlayScene::Terminate() {
    AudioHelper::StopSample(bgmInstance);
    AudioHelper::StopSample(deathBGMInstance);
    deathBGMInstance = std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE>();
    IScene::Terminate();
}
void PlayScene::Update(float deltaTime) {
    // If we use deltaTime directly, then we might have Bullet-through-paper problem.
    // Reference: Bullet-Through-Paper
    if (isPaused) {
        if (preview)
            preview->Update(deltaTime);
        return;
    }
    Engine::Point originalPos = player->Position;
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
    if (player && !activeTextbox) {
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
    if (!isTransitioning && player)
    {
        const float transitionRange = BlockSize * 1.5f;
        const float bottomEdge = (MapHeight * BlockSize) - BlockSize;
        const float playerRightEdge = player->Position.x + (BlockSize / 2);
        const float playerLeftEdge = player->Position.x - (BlockSize / 2);

        if (MapId == 1 && playerRightEdge >= (MapWidth * BlockSize) - transitionRange &&
            player->Position.y >= bottomEdge + 10) {
            isTransitioning = true;
            transitionAlpha = 0.0f;
            pendingSpawnPoint = Engine::Point(
                BlockSize,
                player->Position.y
            );
            }
        else if (MapId == 2 && playerLeftEdge <= transitionRange &&
                 player->Position.y >= bottomEdge + 10) {
            isTransitioning = true;
            transitionAlpha = 0.0f;
            pendingSpawnPoint = Engine::Point(
                (MapWidth * BlockSize) - BlockSize,
                player->Position.y
            );
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

    if (SpeedMult == 0)
        AudioHelper::StopSample(deathBGMInstance);
    if (deathCountDown == -1 && lives > 0) {
        AudioHelper::StopSample(deathBGMInstance);
    }
    if (SpeedMult == 0)
        deathCountDown = -1;

    for (int i = 0; i < SpeedMult; i++) {
        IScene::Update(deltaTime);
        // Check if we should create new enemy.


        // Compensate the time lost.
    }
    if (preview) {
        preview->Position = Engine::GameEngine::GetInstance().GetMousePosition();
        preview->Update(deltaTime);
    }
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
    return collision;
}

void PlayScene::Draw() const {
    IScene::Draw();
    int screenW = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int screenH = Engine::GameEngine::GetInstance().GetScreenSize().y;
    if (DebugMode) {
        // Draw reverse BFS distance on all reachable blocks.
        for (int i = 0; i < MapHeight; i++) {
            for (int j = 0; j < MapWidth; j++) {
                if (mapDistance[i][j] != -1) {
                    // Not elegant nor efficient, but it's quite enough for debugging.
                    Engine::Label label(std::to_string(mapDistance[i][j]), "pirulen.ttf", 32, (j + 0.5) * BlockSize, (i + 0.5) * BlockSize);
                    label.Anchor = Engine::Point(0.5, 0.5);
                    label.Draw();
                }
            }
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
    IScene::OnMouseDown(button, mx, my);
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
    if (keyCode == ALLEGRO_KEY_ESCAPE)
    {
        isPaused = !isPaused;
        if (UIGroup) {
            SetPauseUIVisible(isPaused);
        }
        return;
    }
    if (!activeTextbox) {
        keyDown.insert(keyCode);
        if (player) player->OnKeyDown(keyCode);
    }

    if (isPaused) return;

    if (keyCode == ALLEGRO_KEY_TAB) {
        DebugMode = !DebugMode;
    }

    if (keyCode == ALLEGRO_KEY_B) {
        SaveState("enemy1");
        Engine::GameEngine::GetInstance().ChangeScene("battle");
    }


    // std::cout << interactable << std::endl;
    if (keyCode == ALLEGRO_KEY_E && interactable) {

        if (!activeTextbox) {
            RenderTextbox("intro");
        }
    }
    if (activeTextbox) {
        if (keyCode == ALLEGRO_KEY_ENTER || keyCode == ALLEGRO_KEY_SPACE) {
            if (!nextDiag.empty()) {
                UIGroup->RemoveObject(activeTextbox->GetObjectIterator());
                RenderTextbox(nextDiag);
            } else {
                UIGroup->RemoveObject(activeTextbox->GetObjectIterator());
                activeTextbox = nullptr;
                nameDiag = "";
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
            case 'T': mapData.push_back(5); break;  //Player spawned when change map
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
    for (int i = 0; i < MapHeight; i++) {
        for (int j = 0; j < MapWidth; j++)
        {
            const int num = mapData[i * MapWidth + j];
            switch (num) {
                case 0:  // Dirt
                    mapState[i][j] = TILE_DIRT;
                    TileMapGroup->AddNewObject(new Engine::Image("play/dirt.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 1:  // Floor
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("play/floor.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    break;
                case 2: // NPC1
                {

                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("play/floor.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    Npc *npc1 = new Npc("play/enemy-10.png", j * BlockSize + BlockSize/2, i * BlockSize + BlockSize/2, "npc1");
                    NpcGroup->AddNewObject(npc1);
                    break;
                }
                case 3: // NPC2
                {
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("play/floor.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    Npc *npc2 = new Npc("play/enemy-9.png", j * BlockSize + BlockSize/2, i * BlockSize + BlockSize/2, "npc2");
                    NpcGroup->AddNewObject(npc2);
                    break;
                }
                case 4:  // Spawn point
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("play/floor.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    spawnPoint = Engine::Point(j * BlockSize + BlockSize/2, i * BlockSize + BlockSize/2);
                    break;

                case 5: //Teleport point
                    mapState[i][j] = TILE_FLOOR;
                    TileMapGroup->AddNewObject(new Engine::Image("play/floor.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
                    teleportPoint = Engine::Point(j * BlockSize + BlockSize/2, i * BlockSize + BlockSize/2);
                    break;
            }
        }
    }
    if (!GameData::getInstance().spawned) {
        GameData::getInstance().spawned = true;
        GameData::getInstance().playerPosX = spawnPoint.x;
        GameData::getInstance().playerPosY = spawnPoint.y;
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
    std::string text = data["dialogue"][nameDiag][key]["text"];
    nextDiag = data["dialogue"][nameDiag][key]["next"];


    // std::cout << std::filesystem::current_path() << std::endl;


    activeTextbox = new Engine::Textbox(text,ScreenSize);
    UIGroup->AddNewObject(activeTextbox);
    jsonFileStream.close();
}

void PlayScene::SaveState(std::string enemyName) {
    GameData::getInstance().playerHp = player->getHp();
    GameData::getInstance().playerPosX = player->Position.x;
    GameData::getInstance().playerPosY = player->Position.y;
    GameData::getInstance().currentEnemy = enemyName;
}

void PlayScene::OnBGMSlideValueChanged(float value) {
    if (bgmInstance) {
        al_set_sample_instance_gain(bgmInstance.get(), value);
    }
    AudioHelper::BGMVolume = value;
}

void PlayScene::OnSFXSlideValueChanged(float value) {
    AudioHelper::SFXVolume = value;
    if (deathBGMInstance) {
        al_set_sample_instance_gain(deathBGMInstance.get(), value);
    }
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
    Engine::GameEngine::GetInstance().ChangeScene("stage-select");
}
