#pragma once
#include <SDL.h>
#include <string>
#include "../../include/Text.h"

/* -------------------------------------------------------------------------
 * MessageBox
 *
 * Componente simples de UI para exibir mensagens temporárias na tela:
 * - message_ guarda o texto atual
 * - timer_ controla por quanto tempo a mensagem fica visível
 * - método 'show()' ativa a mensagem por X segundos
 * - método 'update()' decremente o tempo restante
 * - método 'render()' desenha a mensagem se ela estiver ativa
 * ------------------------------------------------------------------------- */
class MessageBox {
public:
    // Exibe nova mensagem por uma duração específica
    void show(const std::string& msg, float duration = 2.0f) {
        message_ = msg;
        timer_   = duration;
        visible_ = true;
    }

    // Atualiza o timer; oculta a mensagem quando expira
    void update(float dt) {
        if (!visible_) return;
        timer_ -= dt;
        if (timer_ <= 0.f) {
            visible_ = false;
        }
    }

    // Desenha a mensagem na posição desejada
    void render(SDL_Renderer* r, Text* text, int x, int y) {
        if (!visible_ || !text) return;
        SDL_Color white{240, 240, 255, 255};
        text->draw(r, message_, x, y, white);
    }

    bool isVisible() const { return visible_; }

private:
    std::string message_;  // texto exibido
    float timer_ = 0.f;     // tempo restante
    bool visible_ = false;  // flag se deve ser desenhada
};
