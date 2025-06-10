#ifndef LOSESCENE_HPP
#define LOSESCENE_HPP

#include <memory>
#include <vector>
#include <allegro5/allegro_audio.h>
#include "Engine/IScene.hpp"
#include "UI/Component/Label.hpp"
#include "UI/Component/ImageButton.hpp"

class LoseScene final : public Engine::IScene {
private:
    std::vector<ALLEGRO_BITMAP*> animationFrames;
    std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE> bgmInstance;

    int currentFrame;
    float currentFrameTime;
    float frameTime;
    int framesCount;

    bool animationComplete = false;
    bool uiShown = false;
    Engine::Label* loseLabel = nullptr;
    Engine::ImageButton* backButton = nullptr;
    Engine::Label* backLabel = nullptr;

    void ShowUI();

public:
    void Initialize() override;
    void Terminate() override;
    void Update(float deltaTime) override;
    void Draw() const override;
    void BackOnClick(int stage);
};

#endif // LOSESCENE_HPP