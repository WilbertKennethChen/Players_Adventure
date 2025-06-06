//
// Created by willy on 2025/6/6.
//

#include "DoorTransitionScene.hpp"
#include "Engine/GameEngine.hpp"
#include "UI/Component/Image.hpp"
#include "Engine/LOG.hpp"

void DoorTransitionScene::Initialize() {
    Engine::LOG(Engine::INFO) << "DoorTransitionScene Initialize";

    // Initialize state
    currentState = State::DOOR_OPEN_PAUSE;
    timer = 1.5f; // Wait for 1.5 seconds with the door open

    // Get screen dimensions
    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;

    // 1. Add the OPEN door background image.
    // This scene starts immediately after the keypress, so we show the result.
    AddNewObject(new Engine::Image("menu/open_door.png", 0, 0, w, h, 0, 0));

    // 2. Create a black sprite that covers the whole screen for the fade effect.
    // It starts fully transparent.
    fadeOverlay = new Engine::Sprite("menu/black_pixel.png", 0, 0, w, h, 0, 0);
    fadeOverlay->Tint.a = 0; // Start transparent
    AddNewObject(fadeOverlay);
}

void DoorTransitionScene::Update(float deltaTime) {
    IScene::Update(deltaTime); // Update all objects in the scene

    switch (currentState) {
        case State::DOOR_OPEN_PAUSE:
            // Countdown the timer
            timer -= deltaTime;
            if (timer <= 0) {
                // When timer is up, start fading out.
                Engine::LOG(Engine::INFO) << "Pause finished. Fading out.";
                currentState = State::FADING_OUT;
                timer = 1.0f; // Fade will take 1 second
            }
            break;

        case State::FADING_OUT:
            timer -= deltaTime;
            if (timer <= 0) {
                // Fade is complete, change to the start scene.
                Engine::LOG(Engine::INFO) << "Fade complete. Changing to StartScene.";
                Engine::GameEngine::GetInstance().ChangeScene("start");
            } else {
                // Calculate and update the alpha value for the fade effect.
                // The progress goes from 0 to 1 as the timer goes from 1 to 0.
                float progress = 1.0f - (timer / 1.0f);
                fadeOverlay->Tint.a = static_cast<unsigned char>(progress * 255);
            }
            break;
    }
}