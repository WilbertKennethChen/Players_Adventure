#ifndef SettingsScene_HPP
#define SettingsScene_HPP
#include <memory>

#include "Engine/IScene.hpp"
#include <allegro5/allegro_audio.h>

class SettingsScene final : public Engine::IScene {
private:
    std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE> bgmInstance;
    std::string waitingForKey;
    std::unordered_map<std::string, Engine::Label*> keyBindLabels;
    void UpdateKeyBindingLabel(const std::string& action);

public:
    explicit SettingsScene() = default;
    void Initialize() override;
    void Terminate() override;
    void BackOnClick(int stage);
    void BGMSlideOnValueChanged(float value);
    void SFXSlideOnValueChanged(float value);
    void OnKeyDown(int keyCode) override;
    void RebindKey(const std::string& action);
    void CreateKeyBindingButton(const std::string& action, float x, float y);
};

#endif   // SettingsScene_HPP
