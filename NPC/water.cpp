//
// Created by swift on 6/11/2025.
//
#include "water.h"
#include <allegro5/allegro_image.h>
#include <iostream>

WaterTile::WaterTile(std::string basePath, float x, float y, float scale, float frameTime)
    : currentFrame(0), frameTime(frameTime), elapsedTime(0), scale(scale) {
    Position.x = x;
    Position.y = y;

    for (int i = 1; i <= 8; ++i) {
        std::string path = "Resource/images/" + basePath + std::to_string(i) + ".png";
        ALLEGRO_BITMAP* bmp = al_load_bitmap(path.c_str());
        if (bmp)
            frames.push_back(bmp);
        else
            std::cerr << "[WaterTile] Failed to load: " << path << std::endl;
    }

    if (!frames.empty()) {
        Size.x = al_get_bitmap_width(frames[0]) * scale;
        Size.y = al_get_bitmap_height(frames[0]) * scale;
    }

    Anchor = Engine::Point(0, 0);
}

WaterTile::~WaterTile() {
    for (auto* bmp : frames) {
        if (bmp) al_destroy_bitmap(bmp);
    }
}

void WaterTile::Update(float deltaTime) {
    if (frames.size() <= 1) return;
    elapsedTime += deltaTime;
    if (elapsedTime >= frameTime) {
        elapsedTime = 0;
        currentFrame = (currentFrame + 1) % frames.size();
    }
}

void WaterTile::Draw() const {
    if (frames.empty()) return;

    ALLEGRO_BITMAP* frame = frames[currentFrame];
    al_draw_scaled_bitmap(frame, 0, 0,
        al_get_bitmap_width(frame), al_get_bitmap_height(frame),
        Position.x, Position.y,
        Size.x, Size.y, 0);
}
