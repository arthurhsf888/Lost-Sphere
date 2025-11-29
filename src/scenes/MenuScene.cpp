#include "MenuScene.h"
#include "Assets.h"
#include <SDL.h>
#include <array>
#include <string>

// Quantidade de itens do menu (Overworld, Sair)
constexpr int MENU_ITEMS = 2;

// Helper para obter o tamanho lógico atual do renderer
static inline void get_render_size(SDL_Renderer* r, int& w, int& h) {
    SDL_RenderGetLogicalSize(r, &w, &h);
    if (w == 0 || h == 0) SDL_GetRendererOutputSize(r, &w, &h);
}

void MenuScene::handleEvent(const SDL_Event& e) {
    if (e.type == SDL_KEYDOWN) {
        // Navegação simples com ↑ e ↓
        if (e.key.keysym.sym == SDLK_UP)
            selected_ = (selected_ + MENU_ITEMS - 1) % MENU_ITEMS;
        if (e.key.keysym.sym == SDLK_DOWN)
            selected_ = (selected_ + 1) % MENU_ITEMS;

        // Enter confirma a opção
        if (e.key.keysym.sym == SDLK_RETURN) {
            if (selected_ == 0) {
                // Vai para o overworld
                sm_.setActive("overworld");
            } else if (selected_ == 1) {
                // Sair do jogo
                SDL_Event quit{};
                quit.type = SDL_QUIT;
                SDL_PushEvent(&quit);
            }
        }
    }
}

void MenuScene::render(SDL_Renderer* r) {
    // Limpa (o background vai cobrir, mas é seguro limpar antes)
    SDL_SetRenderDrawColor(r, 15, 15, 25, 255);
    SDL_RenderClear(r);

    // Carrega o background apenas uma vez
    if (!bg_) {
        bg_ = loadTexture(r, "assets/backgrounds/menu_bg.png");
    }

    // Desenha o background ocupando toda a área lógica
    int W = 0, H = 0;
    get_render_size(r, W, H);
    if (bg_) {
        SDL_Rect dst{0, 0, W, H};
        SDL_RenderCopy(r, bg_, nullptr, &dst);
    }

    // Overlay levemente escuro para dar contraste aos botões
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 8, 8, 12, 70);
    SDL_Rect overlay{0, 0, W, H};
    SDL_RenderFillRect(r, &overlay);

    // Layout dos botões
    constexpr int BW  = 300; // largura
    constexpr int BH  = 50;  // altura
    constexpr int GAP = 22;  // espaçamento vertical

    const int X = (W - BW) / 2 - 390;  // seu offset customizado
    const int totalH = MENU_ITEMS * BH + (MENU_ITEMS - 1) * GAP;
    const int Y0 = (H - totalH) / 2 + 90;

    std::array<SDL_Rect, MENU_ITEMS> items{
        SDL_Rect{ X, Y0 + 0 * (BH + GAP), BW, BH }, // Overworld
        SDL_Rect{ X, Y0 + 1 * (BH + GAP), BW, BH }  // Sair
    };

    const std::array<std::string, MENU_ITEMS> labels{
        "Iniciar",
        "Sair"
    };

    // Desenha cada botão + texto
    for (int i = 0; i < MENU_ITEMS; ++i) {
        if (i == selected_)
            SDL_SetRenderDrawColor(r, 98, 0, 255, 255);   // botão selecionado
        else
            SDL_SetRenderDrawColor(r, 82, 86, 108, 255);  // botão normal

        SDL_RenderFillRect(r, &items[i]);

        if (text_) {
            const int tx = items[i].x + 16;
            const int ty = items[i].y + (BH / 2 - 10);
            text_->draw(r, labels[i], tx, ty);
        }
    }

    SDL_RenderPresent(r);
}
