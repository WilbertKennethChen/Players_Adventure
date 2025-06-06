//
// Created by willy on 2025/6/6.
//

#ifndef DOORTRANSITIONSCENE_HPP
#define DOORTRANSITIONSCENE_HPP

#include "Engine/IScene.hpp"
#include "Engine/Sprite.hpp"

class DoorTransitionScene final : public Engine::IScene {
private:
    enum class State {
        SHOWING_DOOR,
        FADING_OUT,
        COMPLETED
    };

    State currentState;
    float stateTimer;
    float fadeAlpha;
    Engine::Sprite* fadeOverlay;
    float doorShowTime = 0.8f;  // Time to show door before fading
    float fadeOutTime = 1.2f;   // Time for fade out

public:
    explicit DoorTransitionScene() = default;
    void Initialize() override;
    void Update(float deltaTime) override;
};
#endif //DOORTRANSITIONSCENE_HPP
