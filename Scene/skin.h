#ifndef SKIN_H
#define SKIN_H

#include "Engine/AudioHelper.hpp"
#include <allegro5/allegro_audio.h>
#include "Engine/IScene.hpp"
#include <memory>

class SkinScene final : public Engine::IScene {
private:
    std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE> bgmInstance;
public:
    void Initialize() override;
    void Terminate() override;
    void BackOnClick();
};

#endif // SKIN_H
