#include "BossIntroScene.h"
#include <SDL.h>
#include <string>

void BossIntroScene::handleEvent(const SDL_Event& e) {
    if (e.type == SDL_KEYDOWN) {
        // qualquer tecla entra na batalha
        sm_.setActive("battle");
    }
}

void BossIntroScene::render(SDL_Renderer* r) {
    SDL_SetRenderDrawColor(r, 10, 10, 18, 255);
    SDL_RenderClear(r);

    SDL_Rect panel{ 120, 120, 560, 320 };
    SDL_SetRenderDrawColor(r, 25, 25, 40, 255);
    SDL_RenderFillRect(r, &panel);

    if (text_ && boss_) {
        text_->draw(r, std::string("Chefe: ") + boss_->name(), 150, 160);
        text_->draw(r, boss_->intro(), 150, 200);
        text_->draw(r, "Pressione qualquer tecla para lutar", 150, 380);
    }

    SDL_RenderPresent(r);
}
