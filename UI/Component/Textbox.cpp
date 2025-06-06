//
// Created by Celery on 6/4/2025.
//

#include "Textbox.hpp"
#include <allegro5/allegro_font.h>
#include <memory>
#include <allegro5/allegro_primitives.h>

#include "Engine/IObject.hpp"
#include "Engine/Point.hpp"
#include "Engine/Resources.hpp"


namespace Engine {
    Textbox::Textbox(const std::string &text, Engine::Point SceneSize) {
        float width = 1400;
        float height = 400;
        int x = SceneSize.x  / 16;
        int y = SceneSize.y / 2 - 20;
        this->bg = new Engine::Image("play/textbox_bg.png", x, y, width, height);
        this->text = new Engine::Label(text, "pirulen.ttf", 24, x+50, y+50, 255, 255, 255, 255, 0.5, 0.5,width-70);

    }
    Textbox::~Textbox() {
        delete bg;
        delete text;

    }
    void Textbox::Draw() const {
        bg->Draw();
        text->Draw();
    }
    int Textbox::GetTextWidth() const {
        return al_get_text_width(font.get(), Text.c_str());
    }
    int Textbox::GetTextHeight() const {
        return al_get_font_line_height(font.get());
    }
}