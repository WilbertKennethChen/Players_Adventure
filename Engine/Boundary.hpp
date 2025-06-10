//
// Created by willy on 2025/6/10.
//

#ifndef BOUNDARY_HPP
#define BOUNDARY_HPP
#pragma once
#include "Sprite.hpp"

namespace Engine {
    class Boundary : public Sprite {
    public:
        Boundary(float x, float y, float w, float h)
          : Sprite("", x, y) { // no visual
            Size.x = w; Size.y = h;
            Visible = false;
        }
    };
}

#endif //BOUNDARY_HPP
