#include <functional>
#include <string>
#include "Engine/AudioHelper.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Point.hpp"
#include "LoseScene.hpp"
#include "PlayScene.hpp"
#include "UI/Component/Image.hpp"
#include "UI/Component/ImageButton.hpp"
#include "UI/Component/Label.hpp"
#include "Engine/LOG.hpp"
#include <allegro5/allegro_primitives.h>
#include "Player/GameData.hpp"

void LoseScene::Initialize() {
    al_init_primitives_addon();
    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;
    int halfH = h / 2;

    // Animation settings
    frameTime = 0.625f; // Time between frames
    currentFrameTime = 0;
    currentFrame = 0;
    animationComplete = false;
    uiShown = false;

    std::string baseSkin = GameData::getInstance().playerSkin;
    std::string characterKey;
    int totalFrames = 5;

    if (baseSkin.find("kuo") != std::string::npos) {
        characterKey = "kuo";
    } else {
        characterKey = "silent_wanderer_";
    }
    for (int i = 1; i <= totalFrames; i++) {
        std::string path = "Resource/images/lose/loseframe/" + characterKey + std::to_string(i) + ".png";
        std::cout << path << std::endl;
        ALLEGRO_BITMAP* frame = al_load_bitmap(path.c_str());
        if (frame) {
            animationFrames.push_back(frame);
        } else {
            Engine::LOG(Engine::ERROR) << "Failed to load animation frame: " << path;
        }
    }
    framesCount = animationFrames.size();

    // Create UI elements but don't add them to the scene yet
    loseLabel = new Engine::Label("You Lose :(", "undertale.ttf", 96, halfW, halfH / 4 + 10, 255, 255, 255, 255, 0.5, 0.5);
    backButton = new Engine::ImageButton("stage-select/ui_button.png", "stage-select/ui_button_hovered.png",
                                                     (w / 2) - 127, h - 150, 250, 80);
    backButton->SetOnClickCallback(std::bind(&LoseScene::BackOnClick, this, 2));
    backLabel = new Engine::Label("Back", "undertale.ttf", 55, w / 2, h - 110, 0, 0, 0, 255, 0.5, 0.5);

    bgmInstance = AudioHelper::PlaySample("losesound.mp3", false, AudioHelper::BGMVolume);
}

void LoseScene::ShowUI() {
    if (!uiShown) {
        AddNewObject(loseLabel);
        AddNewControlObject(backButton);
        AddNewObject(backLabel);
        uiShown = true;
    }
}

void LoseScene::Terminate() {
    AudioHelper::StopSample(bgmInstance);
    bgmInstance = std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE>();

    // Clean up animation frames
    for (auto frame : animationFrames) {
        al_destroy_bitmap(frame);
    }
    animationFrames.clear();

    IScene::Terminate();
}

void LoseScene::Update(float deltaTime) {
    if (!animationComplete) {
        currentFrameTime += deltaTime;
        if (currentFrameTime >= frameTime) {
            currentFrameTime = 0;
            if (currentFrame < framesCount - 1) {
                currentFrame++;
            } else {
                animationComplete = true;
                ShowUI(); // Show UI when animation completes
            }
        }
    }

    IScene::Update(deltaTime);
}

void LoseScene::Draw() const {
    // Clear background
    al_clear_to_color(al_map_rgb(0, 0, 0));

    IScene::Draw();
    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;

    float radius = 200.0f;  // Adjust as needed
    al_draw_filled_circle(w / 2, h / 2, radius, al_map_rgb(255, 255, 255));

    if (!animationFrames.empty()) {
        ALLEGRO_BITMAP* frame = animationFrames[currentFrame];
        int frameWidth = al_get_bitmap_width(frame);
        int frameHeight = al_get_bitmap_height(frame);

        float scale = 1.0f;
        float offsetY = 0;

        if (GameData::getInstance().playerSkin.find("kuo") != std::string::npos) {
            scale = 0.75f;
            offsetY = 60;
        }

        float drawWidth = frameWidth * scale;
        float drawHeight = frameHeight * scale;

        al_draw_scaled_bitmap(
            frame,
            0, 0, frameWidth, frameHeight,
            w / 2 - drawWidth / 2,
            h / 2 - drawHeight / 2 - offsetY,
            drawWidth, drawHeight,
            0
        );
    }
}

void LoseScene::BackOnClick(int stage) {
    Engine::GameEngine::GetInstance().ChangeScene("play");
}