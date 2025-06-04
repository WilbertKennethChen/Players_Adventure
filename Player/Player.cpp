    //
    // Created by Celery on 6/4/2025.
    //

    #include <allegro5/allegro_primitives.h>
    #include <allegro5/color.h>
    #include <cmath>
    #include <random>
    #include <string>
    #include <vector>


    #include "Engine/AudioHelper.hpp"
    #include "Engine/GameEngine.hpp"
    #include "Engine/Group.hpp"
    #include "Engine/IScene.hpp"
    #include "Engine/LOG.hpp"
    #include "Scene/PlayScene.hpp"


    #include "Player.hpp"

    Player::Player(std::string img, float x, float y, float hp, float speed): Engine::Sprite(img,x,y),hp(hp),speed(speed) {};



    void Player::HandleKey(int keycode,float dt) {
        float move = speed * dt;
        if (keycode == ALLEGRO_KEY_W) Position.y -= move;
        if (keycode == ALLEGRO_KEY_S) Position.y += move;
        if (keycode == ALLEGRO_KEY_D) Position.x += move;
        if (keycode == ALLEGRO_KEY_A) Position.x -= move;
    }


    void Player::Update(float dt) {

    }


    void Player::Draw() const {
        Sprite::Draw();
    }
