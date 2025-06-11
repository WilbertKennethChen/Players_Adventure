//
// Created by Celery on 6/8/2025.
//

#include "Notes.hpp"

#include <iostream>
#include <bits/ostream.tcc>

#include "Engine/GameEngine.hpp"

Notes::Notes(float x, float y, int lane, float speed) : Engine::Sprite("play/bullet-10.png",x,y),lane(lane),speed(speed) {
   Size.x *= 4;
   std::cout << Size.x << "\n";
   Size.y *= 4;
   std::cout << Size.y << "\n";
};

void Notes::Update(float deltaTime) {
   float move = speed * deltaTime;
   Position.x += move;

}
void Notes::Draw() const{
   Sprite::Draw();
}
