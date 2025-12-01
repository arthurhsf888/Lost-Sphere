#include "BossIntroScene.h"
#include <SDL.h>

/* -------------------------------------------------------------------------
 * Evento da cena de introdução do chefe:
 * - Qualquer tecla pressionada já avança imediatamente para a batalha.
 * ------------------------------------------------------------------------- */
void BossIntroScene::handleEvent(const SDL_Event& e) {
    if (e.type == SDL_KEYDOWN) {
        if (nextSceneId_) sm_.setActive(nextSceneId_);
    }
}

/* -------------------------------------------------------------------------
 * Atualização do cronômetro interno:
 * - Após showSeconds_ segundos, avança automaticamente para a batalha
 *   caso o jogador não tenha apertado nenhuma tecla.
 * ------------------------------------------------------------------------- */
void BossIntroScene::update(float dt) {
    t_ += dt;
    if (t_ >= showSeconds_) {
        if (nextSceneId_) sm_.setActive(nextSceneId_);
    }
}

/* -------------------------------------------------------------------------
 * Renderização da tela inicial do chefe:
 * - Mostra fundo simples
 * - Mensagem narrativa
 * - Nome do chefe atual (boss_->name())
 * - Instrução para apertar qualquer tecla
 * ------------------------------------------------------------------------- */
void BossIntroScene::render(SDL_Renderer* r) {
    SDL_SetRenderDrawColor(r, 10, 8, 14, 255);
    SDL_RenderClear(r);

    if (text_) {
        // Mensagem introdutória narrativa
        text_->draw(r, "Um poderoso oponente surge...", 180, 220);

        // Exibe nome do chefe carregado
        if (boss_) {
            text_->draw(r, std::string("Chefe: ") + boss_->name(), 260, 260);
        }

        // Prompt para o jogador
        text_->draw(r, "Pressione qualquer tecla para lutar", 180, 320);
    }

    SDL_RenderPresent(r);
}
