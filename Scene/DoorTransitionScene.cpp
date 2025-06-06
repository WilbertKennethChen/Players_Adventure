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
    // NEW TIMING: A shorter pause to show the open door, but not for too long.
    timer = 0.7f; // Wait for just over half a second.

    // ... (Get screen dimensions code is fine) ...
    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;

    // ... (Adding open_door.png is fine) ...
    AddNewObject(new Engine::Image("menu/open_door.png", 0, 0, w, h, 0, 0));

    // ... (Creating the fadeOverlay is fine) ...
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
                // NEW TIMING: A slightly longer, smoother fade.
                timer = 1.5f; // Fade will take 1.5 seconds.
            }
            break;

        case State::FADING_OUT:
            timer -= deltaTime;
            if (timer <= 0) {
                // Fade is complete, change to the start scene.
                Engine::LOG(Engine::INFO) << "Fade complete. Changing to StartScene.";
                Engine::GameEngine::GetInstance().ChangeScene("start");
            } else {
                // This logic is perfect. It calculates the fade based on the new timer.
                // The progress goes from 0 to 1 as the timer goes from 1.5 to 0.
                float progress = 1.0f - (timer / 1.5f); // Use the new fade duration here too.
                fadeOverlay->Tint.a = static_cast<unsigned char>(progress * 255);
            }
            break;
    }
}