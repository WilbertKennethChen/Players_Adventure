//
// Created by willy on 2025/6/5.
//

#ifndef PRESTARTSCENE_HPP
#define PRESTARTSCENE_HPP

#include "Engine/IScene.hpp"

// Forward declaration for Engine::Label
namespace Engine {
    class Label;
}

class PreStartScene final : public Engine::IScene {
private:
    Engine::Label* instructionLabel = nullptr; // Pointer to our instruction label
    float animationTime = 0.0f;                // To drive the breathing animation

public:
    explicit PreStartScene() = default;
    void Initialize() override;
    void Terminate() override;
    void Update(float deltaTime) override;
    void OnKeyDown(int keyCode) override;
    void OnMouseDown(int button, int mx, int my) override;
};

#endif //PRESTARTSCENE_HPP