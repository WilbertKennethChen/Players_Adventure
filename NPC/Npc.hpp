//
// Created by Celery on 6/5/2025.
//

#ifndef NPC_HPP
#define NPC_HPP

#include "Engine/Point.hpp"
#include "Engine/Sprite.hpp"

class Npc: public Engine::Sprite {
protected:
    float hp;
    std::string name;
    bool hostile;
public:
    Npc(std::string img, float x, float y, std::string name, bool hostile = false);
    virtual void Update(float dt) override;
    void Draw() const override;
    std::string getName() const;
    bool isHostile() const;
    void setFriendly();
};



#endif //NPC_HPP
