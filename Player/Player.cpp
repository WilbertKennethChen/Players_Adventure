#include <allegro5/allegro_primitives.h>
#include <allegro5/color.h>
#include <cmath>
#include <random>
#include <string>
#include <vector>
#include "Engine/AudioHelper.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Group.hpp"
#include "Engine/IScene.hpp"
#include "Engine/LOG.hpp"
#include "Scene/PlayScene.hpp"
#include "Player.hpp"

Player::Player(std::string img, float x, float y, float hp, float speed,
               int frameWidth, int frameHeight, int frameCount)
    : Engine::Sprite(img, x, y), hp(hp), speed(speed),
      frameWidth(frameWidth), frameHeight(frameHeight),
      currentFrame(0), frameCount(frameCount),
      frameTime(0.1f), elapsedTime(0),
      wasMoving(false), currentAnimationRow(0) {
    Size.x = frameWidth;
    Size.y = frameHeight;
    rowFrameCounts = {2, 4};
    rowFrameTimes = {0.4f, 0.1f};
}


bool facingleft = true;
void Player::Update(float dt) {
    float move = speed * dt;
    bool isMovingNow = false;

    if (keyState[ALLEGRO_KEY_W]) {
        Position.y -= move;
        isMovingNow = true;
    }
    if (keyState[ALLEGRO_KEY_S]) {
        Position.y += move;
        isMovingNow = true;
    }
    if (keyState[ALLEGRO_KEY_D]) {
        Position.x += move;
        isMovingNow = true;
        facingleft = false;
    }
    if (keyState[ALLEGRO_KEY_A]) {
        Position.x -= move;
        isMovingNow = true;
        facingleft = true;
    }

    if (isMovingNow != wasMoving) {
        wasMoving = isMovingNow;
        currentAnimationRow = wasMoving ? 1 : 0;
        currentFrame = 0;
        elapsedTime = 0;
    }

    elapsedTime += dt;
    float currentRowFrameTime = rowFrameTimes[currentAnimationRow];
    if (elapsedTime >= currentRowFrameTime) {
        elapsedTime = 0;
        int maxFrames = rowFrameCounts[currentAnimationRow];
        currentFrame = (currentFrame + 1) % maxFrames;
    }
}


void Player::Draw() const {
    float scaleX = (facingleft ? 1.0f : -1.0f) * (42.0f / frameWidth);
    float scaleY = 46.0f / frameHeight;
    float anchorX = facingleft ? Anchor.x : 1.0f - Anchor.x;
    al_draw_tinted_scaled_rotated_bitmap_region(
     bmp.get(),
     currentFrame * frameWidth,
     currentAnimationRow * frameHeight,
     frameWidth,
     frameHeight,
     Tint,
     Anchor.x * frameWidth,
     Anchor.y * frameHeight,
     Position.x,
     Position.y,
     scaleX,
     scaleY,
     Rotation,
     0
 );

}

void Player::OnKeyDown(int keycode) {
    keyState[keycode] = true;
}

void Player::OnKeyUp(int keycode) {
    keyState[keycode] = false;
}
    float Player::getHp() {
        return hp;
    }
    void Player::getHit(int damage) {
        hp-= damage;
    }

