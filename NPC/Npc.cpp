//
// Created by Celery on 6/5/2025.
//

#include "Npc.hpp"
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

Npc::Npc(std::string img, float x, float y,std::string name,bool hostile) : Engine::Sprite(img,x,y), name(name), hostile(hostile) {};



void Npc::Update(float dt) {

}


void Npc::Draw() const {
    Sprite::Draw();
}
std::string Npc::getName() const {
    return name;
}

bool Npc::isHostile() const {
        return this->hostile;
}

void Npc::setFriendly() {
    hostile = false;
}



