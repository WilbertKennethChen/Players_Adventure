#include "enemynpc.h"
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_primitives.h>
#include <iostream>

EnemyNpc::EnemyNpc(std::string name, std::string baseImgPath, float x, float y, bool hostile, float scale)
    : Npc("play/" + baseImgPath + "1.png", x, y, name, hostile),
      currentFrameIndex(0), frameTime(0.3f), elapsedTime(0) {

    std::string base = "Resource/images/play/" + baseImgPath;

    for (int i = 1; i <= 3; ++i) {
        std::string path = base + std::to_string(i) + ".png";
        ALLEGRO_BITMAP* bmp = al_load_bitmap(path.c_str());
        if (bmp) {
            frames.push_back(bmp);
        } else {
            std::cerr << "[EnemyNpc] Failed to load: " << path << std::endl;
        }
    }

    if (!frames.empty()) {
        Size.x = al_get_bitmap_width(frames[0]) * scale;
        Size.y = al_get_bitmap_height(frames[0]) * scale;
    }

    Anchor.x = 0.5f;
    Anchor.y = 0.5f;
}


EnemyNpc::~EnemyNpc() {
    for (auto* bmp : frames) {
        if (bmp) al_destroy_bitmap(bmp);
    }
}

void EnemyNpc::Update(float deltaTime) {
    if (frames.size() <= 1) return;
    elapsedTime += deltaTime;
    if (elapsedTime >= frameTime) {
        elapsedTime = 0;
        currentFrameIndex = (currentFrameIndex + 1) % frames.size();
    }
}
void EnemyNpc::Draw() const {
    if (frames.empty()) return;
    ALLEGRO_BITMAP* frame = frames[currentFrameIndex];

    float drawScale = 0.23f;
    float drawW = al_get_bitmap_width(frame) * drawScale;
    float drawH = al_get_bitmap_height(frame) * drawScale;
    float drawX = Position.x - drawW / 2;
    float drawY = Position.y - drawH / 2;

    al_draw_scaled_bitmap(frame,
        0, 0,
        al_get_bitmap_width(frame), al_get_bitmap_height(frame),
        drawX, drawY,
        drawW, drawH, 0);
}

