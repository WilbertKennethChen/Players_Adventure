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
#include "Scene/SettingsScene.hpp"
#include "Scene/keybinding.h"

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
    Engine::Point originalPosition = Position;
    Engine::Point movementDirection(0.0f, 0.0f);

    auto& keyManager = KeyBindingManager::GetInstance();

    // Movement handling - track direction
    if (keyState[keyManager.GetKeyForAction("up")]) {
        Position.y -= move;
        isMovingNow = true;
    }
    if (keyState[keyManager.GetKeyForAction("down")]) {
        Position.y += move;
        isMovingNow = true;
    }
    if (keyState[keyManager.GetKeyForAction("right")]) {
         Position.x += move;
        isMovingNow = true;
        facingleft = false;
    }
    if (keyState[keyManager.GetKeyForAction("left")]) {
        Position.x -= move;
        isMovingNow = true;
        facingleft = true;
    }

    // Apply movement
    Position.x += movementDirection.x * move;
    Position.y += movementDirection.y * move;

    // --- IMPROVED COLLISION HANDLING ---
    PlayScene* scene = dynamic_cast<PlayScene*>(Engine::GameEngine::GetInstance().GetActiveScene());
    if (scene) {
        const float pushBackForce = 1.0f; // Increased pushback force

        // Check collision with all collidable objects (including houses)
        bool collided = false;
        Engine::Point collisionResponse(0.0f, 0.0f);

        // Check against environment
        if (scene->CheckPlayerCollision(Position)) {
            collided = true;
            // Push back along movement direction
            collisionResponse.x = -movementDirection.x * pushBackForce;
            collisionResponse.y = -movementDirection.y * pushBackForce;
        }

        // Check against NPCs
        for (auto& npc : scene->NpcGroup->GetObjects()) {
            float dx = Position.x - npc->Position.x;
            float dy = Position.y - npc->Position.y;
            float distance = sqrt(dx * dx + dy * dy);
            float minDistance = scene->BlockSize * 0.8f;

            if (distance < minDistance && distance > 0) {
                collided = true;
                float overlap = minDistance - distance;
                collisionResponse.x += (dx / distance) * overlap;
                collisionResponse.y += (dy / distance) * overlap;
            }
        }

        // Apply collision response
        if (collided) {
            Position.x += collisionResponse.x;
            Position.y += collisionResponse.y;

            // Additional check to prevent getting stuck
            if (scene->CheckPlayerCollision(Position)) {
                Position = originalPosition;
            }
        }

        // Boundary checks
        Position.x = std::max(scene->BlockSize/2.f, std::min(Position.x,
                      (float)(scene->MapWidth * scene->BlockSize - scene->BlockSize/2)));
        Position.y = std::max(scene->BlockSize/2.f, std::min(Position.y,
                      (float)(scene->MapHeight * scene->BlockSize - scene->BlockSize/2)));
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

