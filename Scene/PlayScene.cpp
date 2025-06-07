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

#include "Engine/AudioHelper.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Group.hpp"
#include "Engine/LOG.hpp"
#include "Engine/Resources.hpp"
#include "PlayScene.hpp"
#include "Player/GameData.hpp"

#include <allegro5/allegro_primitives.h>


#include "BattleScene.hpp"
#include "UI/Component/Textbox.hpp"
#include "UI/Component/Label.hpp"

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
    SpeedMult = 1;
    std::cout << mapSwitched << std::endl;
    mapSwitched = false;
    // Add groups from bottom to top.
    AddNewObject(TileMapGroup = new Group());
    // Should support buttons.
    AddNewObject(NpcGroup = new Group());
    // Npc *npc = new Npc("play/enemy-10.png",halfW/2,halfH/2,"npc1");
    // NpcGroup->AddNewObject(npc);
    //
    // npc = new Npc("play/enemy-9.png",halfW * 1.5,halfH/2,"npc2");
    // NpcGroup->AddNewObject(npc);
    ReadMap(Engine::Point(-1, -1));
    float playerhp = GameData::getInstance().playerHp;
    float playerposx = GameData::getInstance().playerPosX;
    float playerposy = GameData::getInstance().playerPosY;
    player = new Player("play/enemy-11.png",playerposx, playerposy,playerhp,200);
    AddNewObject(player);
    std::cout << playerhp << std::endl;


    AddNewControlObject(UIGroup = new Group());

    // ReadEnemyWave();
    // mapDistance = CalculateBFSDistance();
    // ConstructUI();
    // imgTarget = new Engine::Image("play/target.png", 0, 0);
    // imgTarget->Visible = false;
    // preview = nullptr;
    // UIGroup->AddNewObject(imgTarget);
    // Preload Lose Scene
    deathBGMInstance = Engine::Resources::GetInstance().GetSampleInstance("astronomia.ogg");
    Engine::Resources::GetInstance().GetBitmap("lose/benjamin-happy.png");
    // Start BGM.
    bgmId = AudioHelper::PlayBGM("play.ogg");
}
void PlayScene::Terminate() {
    AudioHelper::StopBGM(bgmId);
    AudioHelper::StopSample(deathBGMInstance);
    deathBGMInstance = std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE>();
    IScene::Terminate();
}
void PlayScene::Update(float deltaTime) {
    // If we use deltaTime directly, then we might have Bullet-through-paper problem.
    // Reference: Bullet-Through-Paper
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
        for (auto& key:keyDown) {
            player->HandleKey(key,deltaTime);
        }
        if (player->Position != originalPos) {
            if (CheckPlayerCollision(player->Position)) {
                player->Position = originalPos;
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

    bool collision = false;
    for (auto& npc : NpcGroup->GetObjects()) {
        float dx = player->Position.x - npc->Position.x;
        float dy = player->Position.y - npc->Position.y;
        float distance = sqrt(dx * dx + dy * dy);

        if (distance < BlockSize * 0.8f) {
            collision = true;
            break;
        }
    }

    if (collision) {
        player->Position = originalPos;
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
        // dangerIndicator->Tint.a = 0;
    }
    if (SpeedMult == 0)
        deathCountDown = -1;

    for (int i = 0; i < SpeedMult; i++) {
        IScene::Update(deltaTime);
        // Check if we should create new enemy.


        // Compensate the time lost.

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

    // ReadMap(spawnPosition);
}

bool PlayScene::CheckPlayerCollision(const Engine::Point& newPos) {
    const float playerRadius = BlockSize * 0.4f;
    const float npcRadius = BlockSize * 0.4f;

    for (auto& npc : NpcGroup->GetObjects()) {
        float dx = newPos.x - npc->Position.x;
        float dy = newPos.y - npc->Position.y;
        float distance = sqrt(dx * dx + dy * dy);

        if (distance < (playerRadius + npcRadius)) {
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
}
void PlayScene::OnMouseDown(int button, int mx, int my) {
    IScene::OnMouseDown(button, mx, my);
}
void PlayScene::OnMouseMove(int mx, int my) {
    IScene::OnMouseMove(mx, my);
    const int x = mx / BlockSize;
    const int y = my / BlockSize;

}
void PlayScene::OnMouseUp(int button, int mx, int my) {
    IScene::OnMouseUp(button, mx, my);

    const int x = mx / BlockSize;
    const int y = my / BlockSize;
}
void PlayScene::OnKeyUp(int keyCode) {
    IScene::OnKeyUp(keyCode);
    keyDown.erase(keyCode);
}
void PlayScene::OnKeyDown(int keyCode) {
    IScene::OnKeyDown(keyCode);
    keyDown.insert(keyCode);
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
