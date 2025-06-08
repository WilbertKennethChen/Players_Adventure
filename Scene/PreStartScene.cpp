//
// Created by willy on 2025/6/5.
//

#include "PreStartScene.hpp" // Corresponding header file
#include "Engine/GameEngine.hpp"
#include "Engine/Resources.hpp"
#include "UI/Component/Image.hpp"
#include "UI/Component/Label.hpp" // Make sure this is included
#include "Engine/LOG.hpp"
#include <cmath> // For sin function

void PreStartScene::Initialize() {
    Engine::LOG(Engine::INFO) << "PreStartScene Initialize";
    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;

    // 1. Add background image
    try {
        AddNewObject(new Engine::Image("menu/closed_door.png", 0, 0, w, h, 0, 0));
        Engine::LOG(Engine::INFO) << "PreStartScene background image loaded: menu/closed_door.png";
    } catch (const std::runtime_error& e) {
        Engine::LOG(Engine::ERROR) << "Failed to load menu/closed_door.png: " << e.what();
    }

    // 2. Add instruction label at the bottom-center
    std::string instructionText = "Press any key or click to continue...";
    float fontSize = 36;
    // Initial faint color (e.g., white with 50% alpha)
    // We will animate the alpha, so the initial alpha here is a base.
    // Let's set an initial alpha that's on the lower side of the pulse.
    unsigned char initialAlpha = 128; // 50% transparent, will be modulated

    // Create the label and store it
    instructionLabel = new Engine::Label(instructionText, "undertale.ttf", fontSize,
                                   halfW - 125, h - 40,      // x, y position
                                   255, 255, 255, initialAlpha, // r, g, b, a (white color, with initial alpha)
                                   0.5, 1.0);         // anchorX, anchorY (center, bottom)
    AddNewObject(instructionLabel); // Add it to the scene objects
    Engine::LOG(Engine::INFO) << "PreStartScene instruction label added.";

    animationTime = 0.0f; // Reset animation time
}

void PreStartScene::Terminate() {
    Engine::LOG(Engine::INFO) << "PreStartScene Terminate";
    // instructionLabel will be cleaned up by IScene::Terminate calling Group::Clear
    // if it was added via AddNewObject. So, explicit deletion here is not needed
    // unless you manage it completely separately (which we are not).
    instructionLabel = nullptr; // Good practice to nullify pointers after they are no longer valid
    IScene::Terminate();
}

void PreStartScene::Update(float deltaTime) {
    IScene::Update(deltaTime); // Call base class update first

    animationTime += deltaTime;

    // Breathing effect parameters - ADJUSTED FOR MORE FAINTNESS
    float pulseSpeed = 1.5f;  // How fast the pulse is (radians per second) - keep or adjust as desired
    float minAlpha = 25.0f;  // New faintest level (e.g., around 10% opaque)
    float maxAlpha = 100.0f; // New brightest level (was the old faintest)

    // Calculate alpha using a sine wave:
    float normalizedSin = (sin(animationTime * pulseSpeed) + 1.0f) / 2.0f; // Output: 0.0 to 1.0
    float currentAlphaFloat = minAlpha + normalizedSin * (maxAlpha - minAlpha);

    // Ensure alpha doesn't go out of byte range due to floating point inaccuracies, though unlikely with these values
    currentAlphaFloat = std::max(0.0f, std::min(255.0f, currentAlphaFloat));
    unsigned char currentAlphaByte = static_cast<unsigned char>(currentAlphaFloat);

    if (instructionLabel) {
        // Assuming Label has a way to set its color or Tint
        // If Label inherits from Sprite, it has a Tint member:
        // instructionLabel->Tint = al_map_rgba(255, 255, 255, currentAlphaByte);

        // If Label has a SetColor or similar method:
        // instructionLabel->SetColor(255, 255, 255, currentAlphaByte);

        // Looking at your provided files, Engine::Label constructor takes r,g,b,a
        // and likely stores an ALLEGRO_COLOR. Let's assume it has a public member `Color`
        // or a setter. If not, we might need to re-create the label or modify the Label class.

        // Most likely scenario for your engine, based on Sprite and how Label is constructed:
        // Label probably has a public member `Color` of type `ALLEGRO_COLOR`.
        instructionLabel->Color = al_map_rgba(255, 255, 255, currentAlphaByte);
    }
}

void PreStartScene::OnKeyDown(int keyCode) {
    Engine::LOG(Engine::INFO) << "Key " << keyCode << " pressed in PreStartScene. Transitioning to DoorTransitionScene.";
    Engine::GameEngine::GetInstance().ChangeScene("door-transition"); // <-- CHANGE THIS LINE
}

void PreStartScene::OnMouseDown(int button, int mx, int my) {
    Engine::LOG(Engine::INFO) << "Mouse button " << button << " pressed at (" << mx << "," << my << ") in PreStartScene. Transitioning to DoorTransitionScene.";
    Engine::GameEngine::GetInstance().ChangeScene("door-transition"); // <-- CHANGE THIS LINE
}