
//
// Created by willy on 2025/6/9.
//

#include "Engine/AnimatedSprite.hpp"

#include <iostream>
#include <allegro5/allegro.h>

namespace Engine {
    AnimatedSprite::AnimatedSprite(std::vector<std::string>& imgPaths, float x, float y, float w, float h, float anchorX, float anchorY, float timeSpan) :
        Image("", x, y, w, h, anchorX, anchorY), // Call base constructor with a dummy path
        timeSpan(timeSpan),
        timeTicks(0),
        currentFrameIndex(0) {

        for (const auto& path : imgPaths) {
            std::cout << "Loading path: '" << path << "'" << std::endl;
            if (path.empty() || path.find_first_not_of(" \t\n\r") == std::string::npos) {
                std::cout << "Skipping empty path" << std::endl;
                continue;
            }
            bitmaps.push_back(Resources::GetInstance().GetBitmap(path));
        }

        // The base 'bmp' is not used, but we can set it to the first frame for safety.
        if (!bitmaps.empty()) {
            this->bmp = bitmaps[0];
        }
    }

    void AnimatedSprite::Update(float deltaTime) {
        // Only update if there are frames to animate
        if (bitmaps.empty() || timeSpan <= 0) return;

        timeTicks += deltaTime;
        // Calculate how long each frame should be displayed
        float frameDuration = timeSpan / bitmaps.size();

        // If enough time has passed, move to the next frame
        while (timeTicks >= frameDuration) {
            currentFrameIndex = (currentFrameIndex + 1) % bitmaps.size();
            timeTicks -= frameDuration;
        }
    }

    void AnimatedSprite::Draw() const {
        if (bitmaps.empty()) return;

        // Get the current frame's bitmap
        ALLEGRO_BITMAP* currentBmp = bitmaps[currentFrameIndex].get();

        // Draw the current frame, similar to how Engine::Image does it
        al_draw_scaled_bitmap(currentBmp, 0, 0,
            al_get_bitmap_width(currentBmp), al_get_bitmap_height(currentBmp),
            Position.x - Anchor.x * Size.x, Position.y - Anchor.y * Size.y,
            Size.x, Size.y, 0);
    }
}