#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "Engine/Point.hpp"
#include "Engine/Sprite.hpp"
#include <vector>
#include <unordered_map>
#include <allegro5/allegro5.h>

class Player : public Engine::Sprite {
protected:
    float hp;
    float speed;

    std::vector<ALLEGRO_BITMAP*> idleFrames;
    std::vector<ALLEGRO_BITMAP*> walkFrames;

    int currentFrameIndex;
    float frameTime;
    float elapsedTime;
    bool wasMoving;
    bool isWalking;

    std::unordered_map<int, bool> keyState;
    float drawScale;

public:
    Player(std::string baseImgPath, float x, float y, float hp, float speed);

    void Update(float dt) override;
    void Draw() const override;

    void OnKeyDown(int keycode);
    void OnKeyUp(int keycode);

    float getHp();
    void getHit(int damage);
    void setHp(float newHp);
};

#endif // PLAYER_HPP
