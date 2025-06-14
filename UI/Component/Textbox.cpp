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
    Textbox::Textbox(const std::string &text, const std::string &name, Engine::Point SceneSize) {
        float width = 1400;
        float height = 400;
        int x = SceneSize.x  / 16;
        int y = SceneSize.y / 2 - 20;



        std::string trueName;

        if (name == "npc1")
            trueName = "Wilson";
        else if (name == "npc2")
            trueName = "Bradley";
        else if (name == "enemy1")
            trueName = "Ghost";
        else if (name == "jesterKnight")
            trueName = "Jester Knight";
        else if (name == "groveDad")
            trueName = "Grove Warden";
        else if (name == "groveChild")
            trueName = "Grove Warden Jr";
        else
            trueName = name; // fallback to original name


        this->bg = new Engine::Image("play/textbox_bg.png", x, y, width, height);
        this->nameLabel = new Engine::Label(trueName, "undertale.ttf", 52, x + 50, y + 40, 255, 255, 0, 255, 0.0, 0.5);
        this->text = new Engine::Label(text, "undertale.ttf", 44, x + 50, y + 80, 255, 255, 255, 255, 0.0, 0.0, width - 70);
        this->hintLabel = new Engine::Label("press ENTER / SPACE", "undertale.ttf", 28,
        x + width - 60, y + height - 40, 255, 255, 255, 200, 1.0, 1.0);
    }
    Textbox::~Textbox() {
        delete bg;
        delete text;
        delete nameLabel;
        delete hintLabel;
    }
    void Textbox::Draw() const {
        bg->Draw();
        nameLabel->Draw();
        text->Draw();
        if (text->isDone()) {
            hintLabel->Draw();
        }
    }

    int Textbox::GetTextWidth() const {
        return al_get_text_width(font.get(), Text.c_str());
    }
    int Textbox::GetTextHeight() const {
        return al_get_font_line_height(font.get());
    }
    void Textbox::Update(float deltaTime) {
        text->Update(deltaTime);
    }
    void Textbox::Skip() {
        text->Skip();
    }
    bool Textbox::isDone() const {
        return text->isDone();
    }



}