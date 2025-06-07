//
// Created by Celery on 6/4/2025.
//

#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "Engine/Point.hpp"
#include "Engine/Sprite.hpp"


#include <unordered_map>

class Player : public Engine::Sprite {
protected:
    float hp;
    float speed;

    int frameWidth, frameHeight;
    int currentFrame;
    int frameCount;
    float frameTime;
    float elapsedTime;
    bool wasMoving;
    int currentAnimationRow;

    std::unordered_map<int, bool> keyState;
    std::vector<int> rowFrameCounts;
    std::vector<float> rowFrameTimes;
public:
    Player(std::string img, float x, float y, float hp, float speed,
           int frameWidth, int frameHeight, int frameCount);

    void Update(float dt) override;
    void Draw() const override;

    void OnKeyDown(int keycode);
    void OnKeyUp(int keycode);
};


#endif //PLAYER_HPP
