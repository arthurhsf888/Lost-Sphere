#pragma once
#include <SDL.h>
#include <string>
#include "../../include/Text.h"

class MessageBox {
public:
    void show(const std::string& msg, float duration = 2.0f) {
        message_ = msg;
        timer_   = duration;
        visible_ = true;
    }

    void update(float dt) {
        if (!visible_) return;
        timer_ -= dt;
        if (timer_ <= 0.f) {
            visible_ = false;
        }
    }

    void render(SDL_Renderer* r, Text* text, int x, int y) {
        if (!visible_ || !text) return;
        SDL_Color white{240, 240, 255, 255};
        text->draw(r, message_, x, y, white);
    }

    bool isVisible() const { return visible_; }

private:
    std::string message_;
    float timer_ = 0.f;
    bool visible_ = false;
};
