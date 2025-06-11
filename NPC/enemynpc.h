#ifndef ENEMYNPC_H
#define ENEMYNPC_H
#pragma once
#include "Npc.hpp"
#include <vector>
#include <string>

class EnemyNpc : public Npc {
public:
    EnemyNpc(std::string name, std::string baseImgPath, float x, float y, bool hostile = false, float scale = 0.23f);
    ~EnemyNpc();

    void Update(float deltaTime) override;
    void Draw() const override;

private:
    std::vector<ALLEGRO_BITMAP*> frames;
    int currentFrameIndex;
    float frameTime;
    float elapsedTime;
};

#endif // ENEMYNPC_H
