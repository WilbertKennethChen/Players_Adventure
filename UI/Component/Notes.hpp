//
// Created by Celery on 6/8/2025.
//

#ifndef NOTES_HPP
#define NOTES_HPP
#include "Engine/Sprite.hpp"


class Notes : public Engine::Sprite{
protected:

    float speed;

public:
    bool hit=false;
    int lane;
    Notes(float x, float y, int lane, float speed);
    void Update(float deltaTime) override;
    void Draw() const override;
};



#endif //NOTES_HPP
