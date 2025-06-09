//
// Created by willy on 2025/6/9.
//

#ifndef ANIMATEDSPRITE_HPP
#define ANIMATEDSPRITE_HPP


#include <string>
#include <vector>
#include <memory>
#include <allegro5/bitmap.h>
#include "UI/Component/Image.hpp"
#include "Engine/Resources.hpp"

namespace Engine {
    class AnimatedSprite : public Image {
    protected:
        // Stores all the loaded image frames
        std::vector<std::shared_ptr<ALLEGRO_BITMAP>> bitmaps;
        // Total duration for one full animation loop
        float timeSpan;
        // Time accumulator for the current frame
        float timeTicks;
        // Index of the current frame to display
        int currentFrameIndex;

    public:
        // Constructor
        explicit AnimatedSprite(std::vector<std::string>& imgPaths, float x, float y, float w, float h, float anchorX, float anchorY, float timeSpan);

        // Update the animation frame
        void Update(float deltaTime) override;

        // Draw the current animation frame
        void Draw() const override;
    };
}


#endif //ANIMATEDSPRITE_HPP
