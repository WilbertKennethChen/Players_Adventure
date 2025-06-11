#include <allegro5/allegro_font.h>
#include <memory>
#include <sstream>

#include "Engine/IObject.hpp"
#include "Engine/Point.hpp"
#include "Engine/Resources.hpp"
#include "Label.hpp"

#include <cmath>

namespace Engine {
    Label::Label(const std::string &text, const std::string &font, int fontSize, float x, float y, unsigned char r, unsigned char g, unsigned char b, unsigned char a, float anchorX, float anchorY, float maxWidth) : IObject(x, y, 0, 0, anchorX, anchorY), font(Resources::GetInstance().GetFont(font, fontSize)), Text(text), Color(al_map_rgba(r, g, b, a)),maxWidth(maxWidth) {
    }
    void Label::Draw() const {

        if (maxWidth > 0) {
            std::string line;
            std::string visibleText = Text.substr(0, revealedChars);
            std::stringstream s(visibleText);
            std::string word;
            std::string wrapped;
            while (s >> word) {
                std::string testline = line + (line.empty() ? "" : " ") + word;
                float w = al_get_text_width(font.get(),testline.c_str());
                if (w > maxWidth && !line.empty()) {
                    wrapped += line + "\n";
                    line = word;

                } else {
                    line = testline;
                }
            }
            wrapped += line;
            al_draw_multiline_text(font.get(),Color,Position.x,Position.y,maxWidth,35,0,wrapped.c_str());

        } else {
            al_draw_text(font.get(), Color, Position.x - Anchor.x * GetTextWidth(), Position.y - Anchor.y * GetTextHeight(), 0, Text.c_str());
        }

    }
    int Label::GetTextWidth() const {
        return al_get_text_width(font.get(), Text.c_str());
    }
    int Label::GetTextHeight() const {
        return al_get_font_line_height(font.get());
    }

    void Label::Update(float deltaTime) {
        if (maxWidth > 0) {
            if (revealDone) return;
            revealTimer += deltaTime * revealSpeed;
            size_t toReveal = static_cast<size_t>(revealTimer);
            if (toReveal > revealedChars) {
                revealedChars = toReveal;
                if (revealedChars >= Text.size()) {
                    revealedChars = Text.size();
                    revealDone = true;
                }
            }
        }
    }
    void Label::Skip() {
        revealDone = true;
        revealedChars = Text.size();
    }
    bool Label::isDone() const {
        return revealDone;
    }



}
