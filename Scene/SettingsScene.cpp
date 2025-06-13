#include <allegro5/allegro_audio.h>
#include <functional>
#include <memory>
#include <string>

#include "Engine/AudioHelper.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Point.hpp"
#include "Engine/Resources.hpp"
#include "PlayScene.hpp"
#include "Scene/SettingsScene.hpp"
#include "UI/Component/ImageButton.hpp"
#include "UI/Component/Label.hpp"
#include "UI/Component/Slider.hpp"
#include "keybinding.h"

void SettingsScene::Initialize() {
    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int centerX = w / 2;
    int centerY = h / 2;
    AddNewObject(new Engine::Image("menu/main_menu.png", 0, 0, w, h, 0, 0));
    Engine::ImageButton* btn = new Engine::ImageButton("stage-select/ui_button.png", "stage-select/ui_button_hovered.png",
                                                     centerX - 127, h - 150, 250, 80);
    btn->SetOnClickCallback(std::bind(&SettingsScene::BackOnClick, this, 1));
    AddNewControlObject(btn);
    AddNewObject(new Engine::Label("Back", "undertale.ttf", 55, centerX, h - 110, 0, 0, 0, 255, 0.5, 0.5));
    warningLabel = new Engine::Label("", "undertale.ttf", 28, centerX, 160, 255, 50, 50, 255, 0.5, 0.5);
    AddNewObject(warningLabel);

    // ================== LEFT SIDE: Key Bindings ==================
    int leftSectionX = w / 4;
    AddNewObject(new Engine::Label("Controls", "undertale.ttf", 64, leftSectionX, 80, 255, 255, 255, 255, 0.5, 0.5));

    int spacing = 90;
    int keysStartY = 200;

    // Movement label
    AddNewObject(new Engine::Label("Movement & Interact", "undertale.ttf", 36, leftSectionX, keysStartY, 255, 255, 255, 255, 0.5, 0.5));
    keysStartY += 110;
    int adj = 140;
    CreateKeyBindingButton("up", leftSectionX - adj, keysStartY);
    CreateKeyBindingButton("left", leftSectionX - spacing - adj, keysStartY + spacing);
    CreateKeyBindingButton("down", leftSectionX - adj, keysStartY + spacing);
    CreateKeyBindingButton("right", leftSectionX + spacing - adj, keysStartY + spacing);
    CreateKeyBindingButton("interact", leftSectionX + spacing * 2 + 10, keysStartY + spacing - 50);


    // Minigames label
    keysStartY += spacing * 2 + 50;
    AddNewObject(new Engine::Label("Minigames", "undertale.ttf", 36, leftSectionX, keysStartY, 255, 255, 255, 255, 0.5, 0.5));
    keysStartY += 110;
    CreateKeyBindingButton("note_lane_0", leftSectionX - spacing, keysStartY);
    CreateKeyBindingButton("note_lane_1", leftSectionX, keysStartY);
    CreateKeyBindingButton("note_lane_2", leftSectionX + spacing, keysStartY);


    // ================== RIGHT SIDE: Audio Settings ==================
    int rightSectionX = w * 3 / 4;
    AddNewObject(new Engine::Label("Audio", "undertale.ttf", 64, rightSectionX, 80, 255, 255, 255, 255, 0.5, 0.5));

    int slidersStartY = 350;
    int sliderWidth = 600;

    Slider* sliderBGM = new Slider(rightSectionX - sliderWidth/2, slidersStartY, sliderWidth, 4);
    sliderBGM->SetOnValueChangedCallback(std::bind(&SettingsScene::BGMSlideOnValueChanged, this, std::placeholders::_1));
    AddNewControlObject(sliderBGM);
    AddNewObject(new Engine::Label("BGM Volume", "undertale.ttf", 24, rightSectionX, slidersStartY - 50, 255, 255, 255, 255, 0.5, 0.5));

    Slider* sliderSFX = new Slider(rightSectionX - sliderWidth/2, slidersStartY + 100, sliderWidth, 4);
    sliderSFX->SetOnValueChangedCallback(std::bind(&SettingsScene::SFXSlideOnValueChanged, this, std::placeholders::_1));
    AddNewControlObject(sliderSFX);
    AddNewObject(new Engine::Label("SFX Volume", "undertale.ttf", 24, rightSectionX, slidersStartY + 50, 255, 255, 255, 255, 0.5, 0.5));

    bgmInstance = AudioHelper::PlaySample("wii.mp3", true, AudioHelper::BGMVolume);
    sliderBGM->SetValue(AudioHelper::BGMVolume);
    sliderSFX->SetValue(AudioHelper::SFXVolume);
}

void SettingsScene::Terminate() {
    AudioHelper::StopSample(bgmInstance);
    bgmInstance = std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE>();
    IScene::Terminate();
}
void SettingsScene::BackOnClick(int stage) {
    Engine::GameEngine::GetInstance().ChangeScene("start");
}

void SettingsScene::OnKeyDown(int keyCode) {
    if (waitingForKey.empty()) return;

    for (const auto& pair : keyBindLabels) {
        const std::string& action = pair.first;
        if (action != waitingForKey &&
            KeyBindingManager::GetInstance().GetKeyForAction(action) == keyCode) {
            if (warningLabel) {
                warningLabel->Text = "That key is already assigned!";
                warningTimer = 1.3f;
            }
            return;
            }
    }

    KeyBindingManager::GetInstance().SetKeyForAction(waitingForKey, keyCode);
    UpdateKeyBindingLabel(waitingForKey);
    if (warningLabel) warningLabel->Text = "";
    waitingForKey = "";
}

void SettingsScene::Update(float deltaTime) {
    IScene::Update(deltaTime);

    if (warningTimer > 0.0f) {
        warningTimer -= deltaTime;
        if (warningTimer <= 0.0f && warningLabel) {
            warningLabel->Text = "";
        }
    }
}


void SettingsScene::RebindKey(const std::string& action) {
    waitingForKey = action;
}

void SettingsScene::CreateKeyBindingButton(const std::string& action, float x, float y) {
    int buttonSize = 100;

    auto* btn = new Engine::ImageButton("stage-select/button1.png", "stage-select/button2ok.png",
                                      x - buttonSize/2, y - buttonSize/2, buttonSize, buttonSize);
    // Store the label in a class member variable
    auto* keyLabel = new Engine::Label("", "undertale.ttf", 24, x + 2.5, y, 255, 255, 255, 255, 0.5, 0.5);
    keyBindLabels[action] = keyLabel; // Store in a map

    btn->SetOnClickCallback([this, action]() {
        waitingForKey = action;
    });

    AddNewControlObject(btn);
    AddNewObject(keyLabel);
    UpdateKeyBindingLabel(action); // Initialize label text
}

void SettingsScene::UpdateKeyBindingLabel(const std::string& action) {
    std::string currentKey = al_keycode_to_name(KeyBindingManager::GetInstance().GetKeyForAction(action));
    if (keyBindLabels.find(action) != keyBindLabels.end()) {
        keyBindLabels[action]->Text = "\n(" + currentKey + ")";
    }
}

void SettingsScene::BGMSlideOnValueChanged(float value) {
    AudioHelper::ChangeSampleVolume(bgmInstance, value);
    AudioHelper::BGMVolume = value;
}
void SettingsScene::SFXSlideOnValueChanged(float value) {
    AudioHelper::SFXVolume = value;
}
