#include "SelectSetScene.h"
#include <SDL.h>

void SelectSetScene::handleEvent(const SDL_Event& e) {
    if (e.type == SDL_KEYDOWN) {
        if (e.key.keysym.sym == SDLK_UP)   idx_ = (idx_ + 2) % 3;
        if (e.key.keysym.sym == SDLK_DOWN) idx_ = (idx_ + 1) % 3;

        if (e.key.keysym.sym == SDLK_ESCAPE) sm_.setActive("overworld");

        if (e.key.keysym.sym == SDLK_RETURN) {
            gs_->set = (idx_==0? PlayerSet::Guerreiro : idx_==1? PlayerSet::Mago : PlayerSet::Cacador);
            // reset mini-inventário/recursos para cada luta
            gs_->potions = 2;
            sm_.setActive("battle");
        }
    }
}

void SelectSetScene::render(SDL_Renderer* r) {
    SDL_SetRenderDrawColor(r, 12, 14, 22, 255);
    SDL_RenderClear(r);

    SDL_Rect panel{ 140, 100, 520, 360 };
    SDL_SetRenderDrawColor(r, 25, 28, 42, 255);
    SDL_RenderFillRect(r, &panel);

    const std::array<const char*,3> labels{ "Guerreiro", "Mago", "Caçador" };
    const std::array<const char*,3> desc{
        "HP alto, golpes fisicos fortes.\nCusto ST maior, dano consistente.",
        "Dano explosivo com magia.\nCusto ST/gestao mais exigente.",
        "Golpes rapidos de distancia.\nBoa economia de ST."
      };

    // título
    if (text_) text_->draw(r, "Selecione seu Set (Enter confirma, Esc volta)", 160, 120);

    // itens
    for (int i=0;i<3;i++) {
        SDL_Rect item{ 180, 170 + i*90, 440, 70 };
        if (i==idx_) SDL_SetRenderDrawColor(r, 200, 200, 220, 255);
        else         SDL_SetRenderDrawColor(r, 70, 70, 90, 255);
        SDL_RenderFillRect(r, &item);

        if (text_) {
            text_->draw(r, labels[i], item.x + 12, item.y + 10);
            text_->draw(r, desc[i],   item.x + 12, item.y + 36);
        }
    }

    SDL_RenderPresent(r);
}
