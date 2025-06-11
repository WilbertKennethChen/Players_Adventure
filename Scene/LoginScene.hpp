//
// Created by Celery on 6/10/2025.
//

#ifndef LOGINSCENE_HPP
#define LOGINSCENE_HPP



#include <allegro5/allegro_audio.h>
#include <memory>

#include "Engine/IScene.hpp"

class LoginScene final : public Engine::IScene {
private:
    std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE> bgmInstance;
    bool textBoxUser;
    bool textBoxPassword;

    std::string Username;
    std::string Password;
    Engine::Label* UsernameLabel;
    Engine::Label* PasswordLabel;
    std::unordered_map<std::string, std::string> userPass;

public:
    explicit LoginScene() = default;
    void Initialize() override;
    void Terminate() override;

    void BackOnClick(int stage);
    void LoginOnClick();
    void RegisterOnClick();
    void Draw() const override;
    void OnKeyDown(int keyCode) override;
    void OnMouseDown(int button, int mx, int my) override;
};



#endif //LOGINSCENE_HPP
