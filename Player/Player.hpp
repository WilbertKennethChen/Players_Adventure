//
// Created by Celery on 6/4/2025.
//

#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "Engine/Point.hpp"
#include "Engine/Sprite.hpp"


class Player : public Engine::Sprite{
protected:

    float hp;
    float speed;
public:
    Player(std::string img,float x, float y, float hp, float speed);
    void Update(float dt) override;
    void Draw() const override;
    void HandleKey(int keycode,float dt);
};



#endif //PLAYER_HPP
