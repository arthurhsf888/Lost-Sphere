#include "BossIntroScene.h"
#include <SDL.h>

void BossIntroScene::handleEvent(const SDL_Event& e) {
    // Qualquer tecla/enter/esc avança imediatamente para a batalha
    if (e.type == SDL_KEYDOWN) {
        if (nextSceneId_) sm_.setActive(nextSceneId_);
    }
}

void BossIntroScene::update(float dt) {
    t_ += dt;
    if (t_ >= showSeconds_) {
        if (nextSceneId_) sm_.setActive(nextSceneId_);
    }
}

void BossIntroScene::render(SDL_Renderer* r) {
    SDL_SetRenderDrawColor(r, 10, 8, 14, 255);
    SDL_RenderClear(r);

    if (text_) {
        // Título
        text_->draw(r, "Um poderoso oponente surge...", 180, 220);
        // Nome do chefe (usa boss_->name(), já existente)
        if (boss_) {
            text_->draw(r, std::string("Chefe: ") + boss_->name(), 260, 260);
        }
        text_->draw(r, "Pressione qualquer tecla para lutar", 180, 320);
    }

    SDL_RenderPresent(r);
}
