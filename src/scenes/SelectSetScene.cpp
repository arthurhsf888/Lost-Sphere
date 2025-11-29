#include "SelectSetScene.h"
#include "Assets.h"      // para loadTexture
#include <SDL.h>
#include <array>

void SelectSetScene::handleEvent(const SDL_Event& e) {
    if (e.type == SDL_KEYDOWN) {
        if (e.key.keysym.sym == SDLK_UP)   idx_ = (idx_ + 2) % 3;
        if (e.key.keysym.sym == SDLK_DOWN) idx_ = (idx_ + 1) % 3;

        if (e.key.keysym.sym == SDLK_ESCAPE) {
            sm_.setActive("overworld");
        }

        if (e.key.keysym.sym == SDLK_RETURN) {
            if (!gs_) return;

            // define o set escolhido
            gs_->set = (idx_ == 0 ? PlayerSet::Guerreiro
                         : idx_ == 1 ? PlayerSet::Mago
                                     : PlayerSet::Cacador);

            // reset mini-inventário/recursos para cada luta
            gs_->potions = 2;

            // vai para a batalha escolhida no overworld (fallback se vazio)
            if (!gs_->nextBattleSceneId.empty()) {
                sm_.setActive(gs_->nextBattleSceneId);
            } else {
                sm_.setActive("battle_furia");
            }
        }
    }
}

void SelectSetScene::render(SDL_Renderer* r) {
    SDL_SetRenderDrawColor(r, 12, 14, 22, 255);
    SDL_RenderClear(r);

    // ---------- Painel MAIOR ----------
    // Tela lógica é 1280x720, então faço um painel grande e centralizado-ish
    SDL_Rect panel{ 200, 80, 880, 480 };
    SDL_SetRenderDrawColor(r, 25, 28, 42, 255);
    SDL_RenderFillRect(r, &panel);

    const std::array<const char*,3> labels{ "Guerreiro", "Mago", "Caçador" };
    const std::array<const char*,3> desc{
        "HP alto, golpes fisicos fortes. Custo ST maior, dano consistente.",
        "Dano explosivo com magia. Custo ST/gestao mais exigente.",
        "Golpes rapidos de distancia. Boa economia de ST."
    };

    if (text_) {
        text_->draw(r,
            "Selecione seu Set (Enter confirma, Esc volta)",
            panel.x + 40,
            panel.y + 20
        );
    }

    // ---------- Ícones das classes (carregados uma vez) ----------
    static bool iconsLoaded = false;
    static SDL_Texture* icons[3] = { nullptr, nullptr, nullptr };

    if (!iconsLoaded) {
        icons[0] = loadTexture(r, "assets/sprites/classes/guerreiro.png");
        icons[1] = loadTexture(r, "assets/sprites/classes/mago.png");
        icons[2] = loadTexture(r, "assets/sprites/classes/cacador.png");
        iconsLoaded = true;
    }

    // ---------- Itens de seleção MAIORES ----------
    const int itemH   = 110;
    const int itemGap = 20;
    const int itemX   = panel.x + 30;
    const int itemW   = panel.w - 60;
    const int startY  = panel.y + 70;

    const int iconSize = 72;  // tamanho do ícone (quadrado)

    for (int i = 0; i < 3; ++i) {
        SDL_Rect item{
            itemX,
            startY + i * (itemH + itemGap),
            itemW,
            itemH
        };

        if (i == idx_) SDL_SetRenderDrawColor(r, 98, 0, 255, 255);
        else           SDL_SetRenderDrawColor(r, 70, 70, 90, 255);
        SDL_RenderFillRect(r, &item);

        // ícone à esquerda
        if (icons[i]) {
            SDL_Rect iconDst{
                item.x + 16,
                item.y + (itemH - iconSize) / 2,
                iconSize,
                iconSize
            };
            SDL_RenderCopy(r, icons[i], nullptr, &iconDst);

            // posição base do texto depois do ícone
            if (text_) {
                int textX = iconDst.x + iconDst.w + 16;
                int titleY = item.y + 18;
                int descY  = item.y + 50;

                text_->draw(r, labels[i], textX, titleY);
                text_->draw(r, desc[i],   textX, descY);
            }
        } else if (text_) {
            // fallback se não carregar o ícone: só texto, mais pra esquerda
            int textX = item.x + 16;
            int titleY = item.y + 18;
            int descY  = item.y + 50;

            text_->draw(r, labels[i], textX, titleY);
            text_->draw(r, desc[i],   textX, descY);
        }
    }

    SDL_RenderPresent(r);
}
