#include <allegro5/allegro_primitives.h>
#include <allegro5/color.h>
#include <cmath>
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
#include "Engine/Allegro5Exception.hpp"

bool facingleft = true;

Player::Player(std::string baseImgPath, float x, float y, float hp, float speed)
    : Engine::Sprite(baseImgPath + "1.png", x, y), hp(hp), speed(speed),
      currentFrameIndex(0), frameTime(0.2f), elapsedTime(0),
      wasMoving(false), isWalking(false) {

    if (baseImgPath.empty()) {
        throw Engine::Allegro5Exception("Base image path cannot be empty");
    }
    std::string base = "Resource/images/" + baseImgPath;

    // Load idle frames (1–3)
    for (int i = 1; i <= 3; ++i) {
        std::string path = base + std::to_string(i) + ".png";
        ALLEGRO_BITMAP* bmp = al_load_bitmap(path.c_str());
        if (bmp) {
            idleFrames.push_back(bmp);
        } else {
            std::cerr << "[ERROR] Failed to load idle frame: " << path << std::endl;
        }
    }

    // Load walking frames (4–6)
    for (int i = 4; i <= 6; ++i) {
        std::string path = base + std::to_string(i) + ".png";
        ALLEGRO_BITMAP* bmp = al_load_bitmap(path.c_str());
        if (bmp) {
            walkFrames.push_back(bmp);
        } else {
            std::cerr << "[ERROR] Failed to load walking frame: " << path << std::endl;
        }
    }


    // Assume all frames are same size
    if (!idleFrames.empty()) {
        Size.x = al_get_bitmap_width(idleFrames[0]);
        Size.y = al_get_bitmap_height(idleFrames[0]);
    }
}

void Player::Update(float dt) {
    float move = speed * dt;
    bool isMovingNow = false;
    Engine::Point originalPosition = Position;

    auto& keyManager = KeyBindingManager::GetInstance();

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
        facingleft = true;
    }
    if (keyState[keyManager.GetKeyForAction("left")]) {
        Position.x -= move;
        isMovingNow = true;
        facingleft = false;
    }

    PlayScene* scene = dynamic_cast<PlayScene*>(Engine::GameEngine::GetInstance().GetActiveScene());
    if (scene) {
        const float pushBackForce = 1.0f;
        bool collided = false;
        Engine::Point collisionResponse(0.0f, 0.0f);

        if (scene->CheckPlayerCollision(Position)) {
            collided = true;
            collisionResponse = Engine::Point(originalPosition.x - Position.x, originalPosition.y - Position.y);
        }

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

        if (collided) {
            Position.x += collisionResponse.x;
            Position.y += collisionResponse.y;
            if (scene->CheckPlayerCollision(Position)) {
                Position = originalPosition;
            }
        }

        Position.x = std::max(scene->BlockSize / 2.f, std::min(Position.x, (float)(scene->MapWidth * scene->BlockSize - scene->BlockSize / 2)));
        Position.y = std::max(scene->BlockSize / 2.f, std::min(Position.y, (float)(scene->MapHeight * scene->BlockSize - scene->BlockSize / 2)));
    }

    if (isMovingNow != wasMoving) {
        wasMoving = isMovingNow;
        currentFrameIndex = 0;
        elapsedTime = 0;
        isWalking = isMovingNow;
    }

    elapsedTime += dt;
    std::vector<ALLEGRO_BITMAP*>& currentFrames = isWalking ? walkFrames : idleFrames;
    if (elapsedTime >= frameTime && !currentFrames.empty()) {
        elapsedTime = 0;
        currentFrameIndex = (currentFrameIndex + 1) % currentFrames.size();
    }
}

void Player::Draw() const {
    std::vector<ALLEGRO_BITMAP*> frames = isWalking ? walkFrames : idleFrames;
    if (frames.empty()) return;
    ALLEGRO_BITMAP* frame = frames[currentFrameIndex];
    float drawScale = 0.2f;
    float drawW = Size.x * drawScale;
    float drawH = Size.y * drawScale;
    float drawX = Position.x - drawW / 2;
    float drawY = Position.y - drawH / 2;
    float scaleX = facingleft ? 1.0f : -1.0f;

    if (!facingleft) drawX += drawW;

    al_draw_scaled_bitmap(frame,
        0, 0, Size.x, Size.y,
        drawX, drawY,
        drawW * scaleX, drawH, 0);
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
    hp -= damage;
}
