//
// Created by Celery on 6/8/2025.
//

#include "Notes.hpp"

#include "Engine/GameEngine.hpp"

Notes::Notes(float x, float y, int lane, float speed) : Engine::Sprite("play/bullet-10.png",x,y),lane(lane),speed(speed) {
   Size.x *= 4;
   Size.y *= 4;
};

void Notes::Update(float deltaTime) {
   float move = speed * deltaTime;
   Position.x += move;

}
void Notes::Draw() const{
   Sprite::Draw();
}
