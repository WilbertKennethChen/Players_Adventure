//
// Created by willy on 2025/6/6.
//

#include "DoorTransitionScene.hpp"
#include "Engine/GameEngine.hpp"
#include "UI/Component/Image.hpp"
#include "Engine/LOG.hpp"

void DoorTransitionScene::Initialize() {
    Engine::LOG(Engine::INFO) << "DoorTransitionScene Initialize";

    currentState = State::SHOWING_DOOR;
    stateTimer = doorShowTime;
    fadeAlpha = 0.0f;

    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;

    // Add the OPEN door background image
    AddNewObject(new Engine::Image("menu/open_door.png", 0, 0, w, h, 0, 0));

    // Create black overlay for fading (initially transparent)
    fadeOverlay = new Engine::Sprite("menu/black_pixel.png", 0, 0, w, h, 0, 0);
    // Set initial alpha to 0 (completely transparent)
    fadeOverlay->Tint = al_map_rgba(255, 255, 255, 0);
    AddNewObject(fadeOverlay);
}

void DoorTransitionScene::Update(float deltaTime) {
    IScene::Update(deltaTime);

    stateTimer -= deltaTime;

    switch (currentState) {
        case State::SHOWING_DOOR:
            if (stateTimer <= 0) {
                currentState = State::FADING_OUT;
                stateTimer = fadeOutTime;
            }
            break;

        case State::FADING_OUT:
        {
            // Calculate fade progress (0 to 1)
            float fadeProgress = 1.0f - (stateTimer / fadeOutTime);

            // Clamp fadeProgress to [0, 1] to avoid any edge cases
            fadeProgress = std::max(0.0f, std::min(1.0f, fadeProgress));

            // Apply easing function for smoother fade (quadratic ease-in)
            fadeAlpha = fadeProgress * fadeProgress;

            // Convert to alpha value (0-255 range)
            unsigned char alphaValue = static_cast<unsigned char>(fadeAlpha * 255);

            // Update the overlay tint
            fadeOverlay->Tint = al_map_rgba(255, 255, 255, alphaValue);

            if (stateTimer <= 0) {
                currentState = State::COMPLETED;
                Engine::LOG(Engine::INFO) << "Transition finished. Changing to StartScene.";
                Engine::GameEngine::GetInstance().ChangeScene("start");
            }
            break;
        }

        case State::COMPLETED:
            // Do nothing
            break;
    }
}
