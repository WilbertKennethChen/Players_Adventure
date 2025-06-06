//
// Created by willy on 2025/6/6.
//

#ifndef DOORTRANSITIONSCENE_HPP
#define DOORTRANSITIONSCENE_HPP

#include "Engine/IScene.hpp"
#include "Engine/Sprite.hpp"

class DoorTransitionScene final : public Engine::IScene {
private:
    // An enum to keep track of what phase of the animation we are in.
    enum class State {
        DOOR_OPEN_PAUSE,
        FADING_OUT
    };

    State currentState;
    float timer;
    Engine::Sprite* fadeOverlay; // A black sprite we'll use for the fade effect.

public:
    explicit DoorTransitionScene() = default;
    void Initialize() override;
    void Update(float deltaTime) override;
};


#endif //DOORTRANSITIONSCENE_HPP
