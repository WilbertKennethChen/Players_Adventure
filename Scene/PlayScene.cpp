#include <algorithm>
#include <allegro5/allegro.h>
#include <cmath>
#include <fstream>
#include <functional>
#include <memory>
#include <queue>
#include <string>
#include <vector>
#include "Player/Player.hpp"


#include "Engine/AudioHelper.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Group.hpp"
#include "Engine/LOG.hpp"
#include "Engine/Resources.hpp"
#include "PlayScene.hpp"

#include "UI/Component/Textbox.hpp"
#include "UI/Component/Label.hpp"

// TODO HACKATHON-4 (1/3): Trace how the game handles keyboard input.
// TODO HACKATHON-4 (2/3): Find the cheat code sequence in this file.
// TODO HACKATHON-4 (3/3): When the cheat code is entered, a plane should be spawned and added to the scene.
// TODO HACKATHON-5 (1/4): There's a bug in this file, which crashes the game when you win. Try to find it.
// TODO HACKATHON-5 (2/4): The "LIFE" label are not updated when you lose a life. Try to fix it.

bool PlayScene::DebugMode = false;
const std::vector<Engine::Point> PlayScene::directions = { Engine::Point(-1, 0), Engine::Point(0, -1), Engine::Point(1, 0), Engine::Point(0, 1) };
const int PlayScene::MapWidth = 20, PlayScene::MapHeight = 13;
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

Engine::Point PlayScene::GetClientSize() {
    return Engine::Point(MapWidth * BlockSize, MapHeight * BlockSize);
}
void PlayScene::Initialize() {
    keyDown.clear();
    activeTextbox = nullptr;
    mapState.clear();
    keyStrokes.clear();
    ticks = 0;
    deathCountDown = -1;
    lives = 10;
    money = 150;
    SpeedMult = 1;
    int halfW = Engine::GameEngine::GetInstance().GetScreenSize().x / 2;
    int halfH = Engine::GameEngine::GetInstance().GetScreenSize().y / 2;

    // Add groups from bottom to top.
    AddNewObject(TileMapGroup = new Group());
    // AddNewObject(GroundEffectGroup = new Group());
    AddNewObject(DebugIndicatorGroup = new Group());
    // AddNewObject(TowerGroup = new Group());
    // AddNewObject(EnemyGroup = new Group());
    // AddNewObject(BulletGroup = new Group());
    // AddNewObject(EffectGroup = new Group());
    // Should support buttons.
    AddNewControlObject(UIGroup = new Group());
    ReadMap();
    // ReadEnemyWave();
    // mapDistance = CalculateBFSDistance();
    // ConstructUI();

    player = new Player("play/enemy-11.png",halfW, halfH,10,200);
    AddNewObject(player);

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

    if (player) {
        for (auto& key:keyDown) {
            player->HandleKey(key,deltaTime);
        }
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
void PlayScene::Draw() const {
    IScene::Draw();
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


    if (keyCode == ALLEGRO_KEY_E) {
        if (!activeTextbox) {
            RenderTextbox("Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book. It has survived not only five centuries, but also the leap into electronic typesetting, remaining essentially unchanged. It was popularised in the 1960s with the release of Letraset sheets containing Lorem Ipsum passages, and more recently with desktop publishing software like Aldus PageMaker including versions of Lorem Ipsum.");
        }
    }
    if (keyCode == ALLEGRO_KEY_ESCAPE) {
        if (activeTextbox) {
            UIGroup->RemoveObject(activeTextbox->GetObjectIterator());
            activeTextbox = nullptr;
        }
    }




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

void PlayScene::ReadMap() {
    std::string filename = std::string("Resource/map") + std::to_string(MapId) + ".txt";
    // Read map file.
    char c;
    std::vector<bool> mapData;
    std::ifstream fin(filename);
    while (fin >> c) {
        switch (c) {
            case '0': mapData.push_back(false); break;
            case '1': mapData.push_back(true); break;
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
    for (int i = 0; i < MapHeight; i++) {
        for (int j = 0; j < MapWidth; j++) {
            const int num = mapData[i * MapWidth + j];
            mapState[i][j] = num ? TILE_FLOOR : TILE_DIRT;
            if (num)
                TileMapGroup->AddNewObject(new Engine::Image("play/floor.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
            else
                TileMapGroup->AddNewObject(new Engine::Image("play/dirt.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
        }
    }
}

void PlayScene::ConstructUI() {
}

void PlayScene::UIBtnClicked(int id) {
    OnMouseMove(Engine::GameEngine::GetInstance().GetMousePosition().x, Engine::GameEngine::GetInstance().GetMousePosition().y);
}

bool PlayScene::CheckSpaceValid(int x, int y) {
    return true;
    // if (x < 0 || x >= MapWidth || y < 0 || y >= MapHeight)
    //     return false;
    // auto map00 = mapState[y][x];
    // mapState[y][x] = TILE_OCCUPIED;
    // std::vector<std::vector<int>> map = CalculateBFSDistance();
    // mapState[y][x] = map00;
    // if (map[0][0] == -1)
    //     return false;
    // for (auto &it : EnemyGroup->GetObjects()) {
    //     Engine::Point pnt;
    //     pnt.x = floor(it->Position.x / BlockSize);
    //     pnt.y = floor(it->Position.y / BlockSize);
    //     if (pnt.x < 0) pnt.x = 0;
    //     if (pnt.x >= MapWidth) pnt.x = MapWidth - 1;
    //     if (pnt.y < 0) pnt.y = 0;
    //     if (pnt.y >= MapHeight) pnt.y = MapHeight - 1;
    //     if (map[pnt.y][pnt.x] == -1)
    //         return false;
    // }
    // // All enemy have path to exit.
    // mapState[y][x] = TILE_OCCUPIED;
    // mapDistance = map;
    // for (auto &it : EnemyGroup->GetObjects())
    //     dynamic_cast<Enemy *>(it)->UpdatePath(mapDistance);
    // return true;
}
// std::vector<std::vector<int>> PlayScene::CalculateBFSDistance() {
//     // Reverse BFS to find path.
//     std::vector<std::vector<int>> map(MapHeight, std::vector<int>(std::vector<int>(MapWidth, -1)));
//     std::queue<Engine::Point> que;
//     // Push end point.
//     // BFS from end point.
//     if (mapState[MapHeight - 1][MapWidth - 1] != TILE_DIRT)
//         return map;
//     que.push(Engine::Point(MapWidth - 1, MapHeight - 1));
//     map[MapHeight - 1][MapWidth - 1] = 0;
//     while (!que.empty()) {
//         Engine::Point p = que.front();
//         que.pop();
//         // TODO PROJECT-1 (1/1): Implement a BFS starting from the most right-bottom block in the map.
//         //               For each step you should assign the corresponding distance to the most right-bottom block.
//         //               mapState[y][x] is TILE_DIRT if it is empty.
//     }
//     return map;
// }

Engine::Point PlayScene::getScreenSize() {
    return Engine::GameEngine::GetInstance().GetScreenSize();
}

void PlayScene::RenderTextbox(std::string text) {
    Engine::Point ScreenSize = getScreenSize();
    activeTextbox = new Engine::Textbox(text,ScreenSize);
    UIGroup->AddNewObject(activeTextbox);
}
