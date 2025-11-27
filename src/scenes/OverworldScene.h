#pragma once
#include "Scene.h"
#include "SceneManager.h"
#include "Assets.h"
#include "SpriteSheet.h"
#include "Anim.h"
#include "Text.h"
#include <vector>
#include "TileMap.h"

class OverworldScene : public Scene {
public:
    explicit OverworldScene(SceneManager& sm, Text* text = nullptr)
      : sm_(sm), text_(text) {}
    ~OverworldScene() override;

    void handleEvent(const SDL_Event& e) override;
    void update(float dt) override;
    void render(SDL_Renderer* r) override;

private:
    // ------------------------------------------------------------
    // Layout da tela (resolução lógica fixa)
    // ------------------------------------------------------------
    static constexpr int VW      = 1280; // virtual width
    static constexpr int VH      = 720;  // virtual height
    static constexpr int BORDER  = 24;   // espessura das paredes
    static constexpr int MARGIN  = 32;   // distância dos portais à borda
    static constexpr int PSIZE   = 56;   // tamanho (quadrado) do portal
    // ------------------------------------------------------------

    TileMap map_;
    bool mapLoaded_ = false;

    enum class Dir { Up=0, Left=1, Down=2, Right=3 };

    SceneManager& sm_;
    Text* text_ = nullptr;

    // posição/velocidade do player
    SDL_FRect player_{ (VW - 32) * 0.5f, (VH - 32) * 0.5f, 32.f, 32.f };
    float speed_ = 160.f;
    float vx_ = 0.f, vy_ = 0.f;

    // colisão (moldura retangular: top/bottom/left/right)
    std::vector<SDL_Rect> walls_{
        { BORDER,               BORDER,               VW - 2*BORDER, BORDER },          // topo
        { BORDER,               VH - 2*BORDER,        VW - 2*BORDER, BORDER },         // base
        { BORDER,               BORDER,               BORDER,          VH - 2*BORDER }, // esquerda
        { VW - 2*BORDER,        BORDER,               BORDER,          VH - 2*BORDER }  // direita
    };

    struct Portal {
        SDL_Rect rect;
        const char* battleSceneId;
        char label;           // rótulo no portal
    };

    // Portais próximos às pontas (topo-esq, topo-dir, base-esq, base-dir)
    std::vector<Portal> portals_{
        { { MARGIN,                MARGIN,                PSIZE, PSIZE }, "battle_furia",    'F' },
        { { VW - MARGIN - PSIZE,   MARGIN,                PSIZE, PSIZE }, "battle_tempo",    'T' },
        { { MARGIN,                VH - MARGIN - PSIZE,   PSIZE, PSIZE }, "battle_silencio", 'S' },
        { { VW - MARGIN - PSIZE,   VH - MARGIN - PSIZE,   PSIZE, PSIZE }, "battle_orgulho",  'O' },
    };

    // helpers de colisão
    static bool aabbIntersect(const SDL_FRect& a, const SDL_Rect& b);
    void resolveCollisions(SDL_FRect& next);

    // direção/animação do player
    Dir  facing_ = Dir::Down;   // última direção
    bool moving_ = false;
    Anim anim_;                 // controla tempo/fps da caminhada

    // sprites
    SpriteSheet  playerSheet_;  // sheet do player (overworld)
    SDL_Texture* texPortal_ = nullptr;

    // util: primeiro índice da linha da direção atual
    int baseIndexForDir(Dir d) const;
};
