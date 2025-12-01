#pragma once
#include "Scene.h"
#include "SceneManager.h"
#include "Assets.h"
#include "SpriteSheet.h"
#include "Anim.h"
#include "Text.h"
#include "TileMap.h"
#include "../GameState.h"
#include "MessageBox.h"
#include <vector>
#include <string>

/* -----------------------------------------------------------------------------
 * OverworldScene
 *
 * Cena onde o jogador explora o mapa:
 * - movimentação WASD
 * - portais para bosses
 * - porta final
 * - overlays de história (intro / ending)
 * - HUD
 * - prompt “Aperte E para entrar no portal”
 *
 * É o hub central do jogo.
 * ----------------------------------------------------------------------------- */
class OverworldScene : public Scene {
public:
    OverworldScene(SceneManager& sm, Text* text, GameState* gs)
      : sm_(sm), text_(text), gs_(gs) {}
    ~OverworldScene() override;

    void handleEvent(const SDL_Event& e) override;   // teclado
    void update(float dt) override;                  // movimentação, colisão, portais
    void render(SDL_Renderer* r) override;           // desenho total

private:
    // Resolução lógica do jogo
    static constexpr int VW = 1280;
    static constexpr int VH = 720;

    // Mapa
    TileMap map_;
    bool mapLoaded_ = false;

    // Direção do player
    enum class Dir { Up=0, Left=1, Down=2, Right=3 };

    // Estado de overlays narrativos
    enum class OverlayState { None, Intro, Ending };

    SceneManager& sm_;
    Text* text_ = nullptr;
    GameState* gs_ = nullptr; // para saber quais bosses já foram derrotados/configurar batalha

    // Posição e tamanho do player (em pixels)
    SDL_FRect player_{ VW - 32.f - 700.f, VH - 32.f - 150.f, 32.f, 32.f };
    float speed_ = 160.f;
    float vx_ = 0.f, vy_ = 0.f;

    // Estrutura auxiliar (não usada atualmente)
    std::vector<SDL_Rect> walls_{};

    // -------------------------------------------------------------------------
    // Definição de portal
    // -------------------------------------------------------------------------
    struct Portal {
        SDL_Rect rect;
        std::string id;            // "furia", "tempo", "silencio", "orgulho"
        const char* battleSceneId; // cena de batalha correspondente
        char label;                // letra desenhada em cima (F, T, S, O)
    };

    // Quatro portais com posições fixas
    std::vector<Portal> portals_{
        { {  50,  35, 56, 56 }, "furia",    "battle_furia",    'F' },
        { { 770,  60, 56, 56 }, "tempo",    "battle_tempo",    'T' },
        { {  50, 470, 56, 56 }, "silencio", "battle_silencio", 'S' },
        { { 800, 450, 56, 56 }, "orgulho",  "battle_orgulho",  'O' },
    };

    // -------------------------------------------------------------------------
    // Porta final (que só abre quando 4 bosses derrotados)
    // -------------------------------------------------------------------------
    SDL_Rect     door_{ 1110, 400, 64, 96 };
    SDL_Texture* doorTex_ = nullptr;

    // Overlays narrativos (intro + ending)
    SDL_Texture* storyIntroTex_  = nullptr;
    SDL_Texture* storyEndingTex_ = nullptr;
    OverlayState overlay_ = OverlayState::Intro;

    // HUD de caveira
    SDL_Texture* skullTex_ = nullptr;

    // Flag para prompt de portal
    bool nearPortal_ = false;

    // -------------------------------------------------------------------------
    // Funções auxiliares de colisão
    // -------------------------------------------------------------------------
    static bool aabbIntersect(const SDL_FRect& a, const SDL_Rect& b);
    bool collidesWithSolidTiles(const SDL_FRect& r) const;
    void resolveCollisions(SDL_FRect& next);

    // -------------------------------------------------------------------------
    // Player: direção, animação e spritesheet
    // -------------------------------------------------------------------------
    Dir  facing_ = Dir::Down;
    bool moving_ = false;
    Anim anim_;

    SpriteSheet playerSheet_;
    SpriteSheet portalSheet_;

    // Mensagens temporárias (como "Portal dissipado", "A porta está trancada")
    MessageBox msgBox_;

    // Música e sons
    bool  musicStarted_ = false;
    float stepTimer_    = 0.f;
    bool  stepToggle_   = false;

    // Converte direção → índice base no spritesheet
    int baseIndexForDir(Dir d) const;
};
