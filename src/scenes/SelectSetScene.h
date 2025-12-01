#pragma once
#include "Scene.h"
#include "SceneManager.h"
#include "Text.h"
#include "../GameState.h"
#include <SDL.h>
#include <array>
#include <string>

/* -----------------------------------------------------------------------------
 * SelectSetScene
 *
 * Tela de escolha de classe do jogador (Guerreiro, Mago, Caçador).
 * Depois que o jogador entra em um portal no Overworld, esta cena aparece para
 * ele escolher o "set" antes de iniciar a batalha.
 *
 * A cena também mostra, à direita, uma dica específica do boss correspondente
 * ao portal que o jogador abriu. Essas imagens são PNGs grandes e trocam
 * dinamicamente ao mudar de portal.
 *
 * Operações principais:
 * - ↑ e ↓ movem o cursor
 * - ENTER confirma classe e inicia batalha
 * - ESC volta para o Overworld
 * ----------------------------------------------------------------------------- */
class SelectSetScene : public Scene {
public:
    SelectSetScene(SceneManager& sm, Text* text, GameState* gs)
        : sm_(sm), text_(text), gs_(gs) {}

    void handleEvent(const SDL_Event& e) override; // navegação e seleção
    void update(float) override {}                 // cena é estática
    void render(SDL_Renderer* r) override;         // painel + dica do boss

private:
    SceneManager& sm_;
    Text*         text_ = nullptr;
    GameState*    gs_   = nullptr;

    // Textura da dica do boss (PNG grande no lado direito)
    SDL_Texture* hintTex_ = nullptr;

    // Identificador do último portal carregado (para saber se recarrega PNG)
    std::string lastHintPortalId_;

    // Índice da classe selecionada
    // 0 = Guerreiro
    // 1 = Mago
    // 2 = Caçador
    int idx_ = 0;
};
