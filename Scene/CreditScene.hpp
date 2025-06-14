//
// Created by Celery on 6/12/2025.
//

#ifndef CREDITSCENE_HPP
#define CREDITSCENE_HPP


#include <allegro5/allegro_audio.h>
#include <list>
#include <memory>
#include <utility>
#include <vector>
#include <set>


#include "Engine/IScene.hpp"
#include "Engine/Point.hpp"
#include "Player/Player.hpp"
#include "UI/Component/Textbox.hpp"
#include "UI/Component/Slider.hpp"
#include <allegro5/allegro_audio.h>
#include <memory>

#include "Engine/IScene.hpp"

class CreditScene final : public Engine::IScene {
private:
    std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE> bgmInstance;
    float scrollY;
    float scrollSpeed;
    Group *CreditGroup;
    std::vector<Engine::Label*> credits;
    std::vector<Engine::Image*> creditsImg;
    std::vector<Engine::Label*> creditsImgTitle;
    std::vector<std::string> imgTitle;
    bool revealStarted;
    float fadeA;
    float fadeDuration;
    bool toggleSpeed;
    bool Done;
    bool Skip;
public:
    explicit CreditScene() = default;
    void Initialize() override;
    void Terminate() override;
    void ResetOnClick();
    void Update(float deltaTime) override;
    void Draw() const override;
    void OnKeyDown(int keyCode) override;
    void BackOnClick();
};



#endif //CREDITSCENE_HPP
