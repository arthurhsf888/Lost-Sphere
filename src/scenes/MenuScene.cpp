#include "MenuScene.h"
#include "Assets.h"
#include <SDL.h>
#include <array>
#include <string>

// Helpers para pegar o tamanho lógico atual (cai para o tamanho real se não houver logical size)
static inline void get_render_size(SDL_Renderer* r, int& w, int& h) {
    SDL_RenderGetLogicalSize(r, &w, &h);
    if (w == 0 || h == 0) SDL_GetRendererOutputSize(r, &w, &h);
}

void MenuScene::handleEvent(const SDL_Event& e) {
    if (e.type == SDL_KEYDOWN) {
        if (e.key.keysym.sym == SDLK_UP)    selected_ = (selected_ + 2) % 3;
        if (e.key.keysym.sym == SDLK_DOWN)  selected_ = (selected_ + 1) % 3;
        if (e.key.keysym.sym == SDLK_RETURN) {
            if (selected_ == 0) sm_.setActive("overworld");
            else if (selected_ == 1) sm_.setActive("battle");
            else if (selected_ == 2) { SDL_Event quit{}; quit.type = SDL_QUIT; SDL_PushEvent(&quit); }
        }
    }
}

void MenuScene::render(SDL_Renderer* r) {
    // Limpa (o BG vai cobrir, mas mantém seguro)
    SDL_SetRenderDrawColor(r, 15, 15, 25, 255);
    SDL_RenderClear(r);

    // Carrega o background uma única vez
    if (!bg_) {
        bg_ = loadTexture(r, "assets/backgrounds/menu_bg.png");
    }

    // Desenha o BG do tamanho da viewport lógica
    int W = 0, H = 0;
    get_render_size(r, W, H);
    if (bg_) {
        SDL_Rect dst{0, 0, W, H};
        SDL_RenderCopy(r, bg_, nullptr, &dst);
    }

    // (Opcional) Overlay sutil para contraste do texto/botões
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 8, 8, 12, 140);
    SDL_Rect overlay{0, 0, W, H};
    SDL_RenderFillRect(r, &overlay);

    // Layout dos botões centralizados
    constexpr int BW = 360;  // largura do botão
    constexpr int BH = 50;   // altura do botão
    constexpr int GAP = 22;  // espaçamento vertical

    const int X = (W - BW) / 2;
    const int totalH = 3*BH + 2*GAP;
    const int Y0 = (H - totalH) / 2;  // centraliza verticalmente

    std::array<SDL_Rect, 3> items{
        SDL_Rect{ X, Y0 + 0*(BH+GAP), BW, BH },
        SDL_Rect{ X, Y0 + 1*(BH+GAP), BW, BH },
        SDL_Rect{ X, Y0 + 2*(BH+GAP), BW, BH }
    };
    const std::array<std::string,3> labels{ "Overworld", "Batalha", "Sair" };

    for (int i = 0; i < 3; ++i) {
        if (i == selected_) SDL_SetRenderDrawColor(r, 205, 206, 220, 255);
        else                SDL_SetRenderDrawColor(r, 82, 86, 108, 255);
        SDL_RenderFillRect(r, &items[i]);

        // texto (alinhamento simples à esquerda com padding)
        if (text_) {
            const int tx = items[i].x + 16;
            const int ty = items[i].y + (BH/2 - 10); // ajuste visual
            text_->draw(r, labels[i], tx, ty);
        }
    }

    SDL_RenderPresent(r);
}
