//
// Created by swift on 6/11/2025.
//

#ifndef WATER_H
#define WATER_H
#pragma once
#include "Engine/IObject.hpp"
#include <allegro5/allegro.h>
#include <vector>
#include <string>

class WaterTile : public Engine::IObject {
public:
    WaterTile(std::string basePath, float x, float y, float scale = 1.0f, float frameTime = 0.6f);
    ~WaterTile();
    void Update(float deltaTime) override;
    void Draw() const override;

private:
    std::vector<ALLEGRO_BITMAP*> frames;
    int currentFrame;
    float frameTime;
    float elapsedTime;
    float scale;
};

#endif //WATER_H
