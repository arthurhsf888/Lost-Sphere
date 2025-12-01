#include "SelectSetScene.h"
#include "Assets.h"
#include "../SoundManager.h"
#include <SDL.h>
#include <array>

// -----------------------------------------------------------------------------
// Instância global do gerenciador de som (definida em main.cpp)
// -----------------------------------------------------------------------------
extern SoundManager gSound;

// -----------------------------------------------------------------------------
// HANDLE EVENT — navegação do menu e confirmação da classe
// -----------------------------------------------------------------------------
void SelectSetScene::handleEvent(const SDL_Event& e) {
    if (e.type == SDL_KEYDOWN) {
        const SDL_Keycode key = e.key.keysym.sym;

        // ---------------------------------------------------------------------
        // Mover cursor (↑ e ↓)
        // ---------------------------------------------------------------------
        if (key == SDLK_UP) {
            idx_ = (idx_ + 2) % 3; // 0→2, 1→0, 2→1 (loop)
            if (gSound.ok()) gSound.playSfx("select_button");
        }
        if (key == SDLK_DOWN) {
            idx_ = (idx_ + 1) % 3; // 0→1→2→0
            if (gSound.ok()) gSound.playSfx("select_button");
        }

        // ---------------------------------------------------------------------
        // ESC → voltar ao overworld
        // ---------------------------------------------------------------------
        if (key == SDLK_ESCAPE) {
            if (gSound.ok()) gSound.playSfx("click_button");
            if (gSound.ok()) gSound.playMusic("overworld_theme", -1);
            sm_.setActive("overworld");
        }

        // ---------------------------------------------------------------------
        // ENTER → confirmar classe e ir para a batalha
        // ---------------------------------------------------------------------
        if (key == SDLK_RETURN) {
            if (!gs_) return;

            if (gSound.ok()) gSound.playSfx("click_button");

            // Define o set de acordo com o índice selecionado
            gs_->set = (idx_ == 0 ? PlayerSet::Guerreiro
                        : idx_ == 1 ? PlayerSet::Mago
                                    : PlayerSet::Cacador);

            gs_->potions = 2;

            // Vai para a batalha correspondente ao portal
            if (!gs_->nextBattleSceneId.empty()) {
                sm_.setActive(gs_->nextBattleSceneId);
            } else {
                sm_.setActive("battle_furia"); // fallback defensivo
            }
        }
    }
}

// -----------------------------------------------------------------------------
// RENDER — desenha o menu de seleção de classe + dica do boss
// -----------------------------------------------------------------------------
void SelectSetScene::render(SDL_Renderer* r) {

    // Fundo sólido
    SDL_SetRenderDrawColor(r, 12, 14, 22, 255);
    SDL_RenderClear(r);

    // -------------------------------------------------------------------------
    // RECARREGAR DICA DO BOSS (painel da direita)
    // Troca automaticamente quando o portal muda
    // -------------------------------------------------------------------------
    if (gs_) {
        const std::string curPortal = gs_->lastPortalId;

        // Caso ainda não exista textura OU o portal mudou
        if (!hintTex_ || curPortal != lastHintPortalId_) {

            // Liberar textura antiga
            if (hintTex_) {
                SDL_DestroyTexture(hintTex_);
                hintTex_ = nullptr;
            }

            lastHintPortalId_ = curPortal;

            // Define o PNG correto
            std::string hintPath = "assets/ui/dica_default.png";

            if (curPortal == "furia")         hintPath = "assets/ui/dica_boss_furia.png";
            else if (curPortal == "silencio") hintPath = "assets/ui/dica_boss_silencio.png";
            else if (curPortal == "tempo")    hintPath = "assets/ui/dica_boss_tempo.png";
            else if (curPortal == "orgulho")  hintPath = "assets/ui/dica_boss_orgulho.png";

            // Carrega textura
            hintTex_ = loadTexture(r, hintPath);
        }
    }

    // -------------------------------------------------------------------------
    // Painel principal (lado esquerdo)
    // -------------------------------------------------------------------------
    SDL_Rect panel{ 80, 80, 720, 480 };
    SDL_SetRenderDrawColor(r, 25, 28, 42, 255);
    SDL_RenderFillRect(r, &panel);

    // Labels das classes
    const std::array<const char*,3> labels{ "Guerreiro", "Mago", "Caçador" };
    const std::array<const char*,3> desc{
        "HP alto, golpes físicos fortes.",
        "Dano explosivo com magia.",
        "Golpes rápidos de distância."
    };

    // Título do painel
    if (text_) {
        text_->draw(
            r,
            "Selecione seu Set (Enter confirma, Esc volta)",
            panel.x + 30,
            panel.y + 20
        );
    }

    // -------------------------------------------------------------------------
    // Ícones das classes — carregados apenas uma vez
    // -------------------------------------------------------------------------
    static bool iconsLoaded = false;
    static SDL_Texture* icons[3] = { nullptr, nullptr, nullptr };

    if (!iconsLoaded) {
        icons[0] = loadTexture(r, "assets/sprites/classes/guerreiro.png");
        icons[1] = loadTexture(r, "assets/sprites/classes/mago.png");
        icons[2] = loadTexture(r, "assets/sprites/classes/cacador.png");
        iconsLoaded = true;
    }

    // -------------------------------------------------------------------------
    // Itens de seleção (cada classe é um retângulo clicável)
    // -------------------------------------------------------------------------
    const int itemH     = 110;
    const int itemGap   = 20;
    const int itemX     = panel.x + 20;
    const int itemW     = panel.w - 40;
    const int startY    = panel.y + 70;
    const int iconSize  = 72;

    for (int i = 0; i < 3; ++i) {

        // Caixa do item
        SDL_Rect item{
            itemX,
            startY + i * (itemH + itemGap),
            itemW,
            itemH
        };

        // Cor de destaque
        if (i == idx_) SDL_SetRenderDrawColor(r, 98, 0, 255, 255);
        else           SDL_SetRenderDrawColor(r, 70, 70, 90, 255);

        SDL_RenderFillRect(r, &item);

        // Ícone da classe
        if (icons[i]) {
            SDL_Rect iconDst{
                item.x + 16,
                item.y + (itemH - iconSize) / 2,
                iconSize,
                iconSize
            };
            SDL_RenderCopy(r, icons[i], nullptr, &iconDst);

            // Textos (nome + descrição)
            if (text_) {
                int tx = iconDst.x + iconDst.w + 16;
                text_->draw(r, labels[i], tx, item.y + 18);
                text_->draw(r, desc[i],   tx, item.y + 50);
            }
        }
    }

    // -------------------------------------------------------------------------
    // Painel da dica do boss — grande PNG ao lado direito
    // -------------------------------------------------------------------------
    if (hintTex_) {
        SDL_Rect dst;
        dst.w = 900;
        dst.h = 700;
        dst.x = 1280 - dst.w + 210;
        dst.y = 0;

        SDL_RenderCopy(r, hintTex_, nullptr, &dst);
    }

    SDL_RenderPresent(r);
}
