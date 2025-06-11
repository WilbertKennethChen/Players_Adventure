#ifndef PLAYSCENE_HPP
#define PLAYSCENE_HPP
#include <allegro5/allegro_audio.h>
#include <list>
#include <memory>
#include <utility>
#include <vector>
#include <set>


#include "Engine/IScene.hpp"
#include "Engine/Point.hpp"
#include "Player/Player.hpp"
#include "UI/Component/Textbox.hpp"
#include "UI/Component/Slider.hpp"

// A simple AABB
struct Box { float x, y, w, h; };
inline std::vector<Box> treeColliders;

class Turret;
namespace Engine {
    class Group;
    class Image;
    class Label;
    class Sprite;
}   // namespace Engine

class PlayScene final : public Engine::IScene {
private:
    enum TileType {
        TILE_DIRT,
        TILE_FLOOR,
        TILE_OCCUPIED,
        TILE_HOUSE,
        TILE_WATER,
        TILE_TREE,
    };
    Slider* sliderBGM;
    Slider* sliderSFX;
    ALLEGRO_SAMPLE_ID bgmId;
    std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE> bgmInstance;
    std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE> deathBGMInstance;

protected:
    int lives;
    int money;
    int SpeedMult;
    Player* player;

public:
    static bool DebugMode;
    static const std::vector<Engine::Point> directions;
    static const int MapWidth, MapHeight;
    static const int BlockSize;
    static const float DangerTime;
    static const Engine::Point SpawnGridPoint;
    static const Engine::Point EndGridPoint;
    static const std::vector<int> code;
    int MapId;
    float ticks;
    float deathCountDown;
    // Map tiles.
    Group *TileMapGroup;
    Group *GroundEffectGroup;
    Group *DebugIndicatorGroup;
    Group *BulletGroup;
    Group *TowerGroup;
    Group *EnemyGroup;
    Group *EffectGroup;
    Group *UIGroup;
    Group *NpcGroup;
    Engine::Textbox *activeTextbox;
    Engine::Label *UIMoney;
    Engine::Label *UILives;
    Engine::Image *imgTarget;
    Engine::Sprite *dangerIndicator;
    Engine::Image* dialogueNpcImage = nullptr;

    Turret *preview;
    std::vector<std::vector<TileType>> mapState;
    std::vector<std::vector<int>> mapDistance;
    std::list<std::pair<int, float>> enemyWaveData;
    std::list<int> keyStrokes;
    static Engine::Point GetClientSize();
    explicit PlayScene() = default;
    void Initialize() override;
    void PauseInitialize();
    void Terminate() override;
    void Update(float deltaTime) override;
    bool CheckPlayerCollision(const Engine::Point& newPos);
    void Draw() const override;
    void ResetScene(Engine::Point spawnPosition);
    void OnMouseDown(int button, int mx, int my) override;
    void OnMouseMove(int mx, int my) override;
    void OnMouseUp(int button, int mx, int my) override;
    void OnKeyDown(int keyCode) override;
    void OnKeyUp(int keyCode) override;
    void Hit();
    int GetMoney() const;
    void EarnMoney(int money);
    void ReadMap(Engine::Point overrideSpawn);
    void ReadEnemyWave();
    void ConstructUI();
    void UIBtnClicked(int id);
    bool CheckSpaceValid(int x, int y);
    void RenderTextbox(std::string text);
    void SaveState(std::string enemyName);
    Engine::Point getScreenSize();
    std::vector<std::vector<int>> CalculateBFSDistance();
    float transitionAlpha = -1.0f;
    bool isTransitioning = false;
    Engine::Point pendingSpawnPoint;
    bool isPaused = false;
    Engine::Label* pauseLabel;
    Engine::Label* continueLabel;
    Engine::Label* exitLabel;
    Engine::ImageButton* retryBtn = nullptr;
    Engine::Label* retryLabel = nullptr;
    Engine::Image* pauseBackground = nullptr;
    Engine::ImageButton* continueBtn = nullptr;
    Engine::ImageButton* exitBtn = nullptr;
    void SetPauseUIVisible(bool visible);
    void OnBGMSlideValueChanged(float value);
    void OnSFXSlideValueChanged(float value);
    Engine::Label* BGMLabel;
    Engine::Label* SFXLabel;
    bool draggingBGM = false;
    bool draggingSFX = false;
    void ContinueOnClick();
    void RetryOnClick();
    void ExitOnClick();

    // void ModifyReadMapTiles();
};
#endif   // PLAYSCENE_HPP
