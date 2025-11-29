#pragma once
#include "Scene.h"
#include "SceneManager.h"
#include "Assets.h"
#include "SpriteSheet.h"
#include "Anim.h"
#include "Text.h"
#include "TileMap.h"
#include "GameState.h"
#include <vector>

class OverworldScene : public Scene {
public:
    explicit OverworldScene(SceneManager& sm, Text* text, GameState* gs)
      : sm_(sm), text_(text), gs_(gs) {}
    ~OverworldScene() override;

    void handleEvent(const SDL_Event& e) override;
    void update(float dt) override;
    void render(SDL_Renderer* r) override;

private:
    // Layout da tela (resolução lógica fixa)
    static constexpr int VW = 1280;
    static constexpr int VH = 720;

    TileMap map_;
    bool mapLoaded_ = false;

    enum class Dir { Up=0, Left=1, Down=2, Right=3 };

    SceneManager& sm_;
    Text* text_ = nullptr;
    GameState* gs_ = nullptr;   // <-- para saber qual batalha abrir depois

    // posição/velocidade do player
    SDL_FRect player_{ VW - 32.f - 700.f, VH - 32.f - 150.f, 32.f, 32.f };
    float speed_ = 160.f;
    float vx_ = 0.f, vy_ = 0.f;

    // (antigas paredes de moldura – atualmente não usadas)
    std::vector<SDL_Rect> walls_{};

    struct Portal {
        SDL_Rect rect;
        const char* battleSceneId;
        char label;           // rótulo no portal
    };

    // Portais em posições fixas
    std::vector<Portal> portals_{
        { {  50,  35, 56, 56 }, "battle_furia",    'F' },
        { { 770,  60, 56, 56 }, "battle_tempo",    'T' },
        { {  50, 470, 56, 56 }, "battle_silencio", 'S' },
        { { 800, 450, 56, 56 }, "battle_orgulho",  'O' },
    };

    // helpers de colisão
    static bool aabbIntersect(const SDL_FRect& a, const SDL_Rect& b);
    void resolveCollisions(SDL_FRect& next);
    bool collidesWithSolidTiles(const SDL_FRect& r) const;

    // direção/animação do player
    Dir  facing_ = Dir::Down;
    bool moving_ = false;
    Anim anim_;

    // sprites
    SpriteSheet  playerSheet_;   // sheet do player (overworld)
    SpriteSheet  portalSheet_;   // sheet dos portais (3x2 de 32x32)

    int baseIndexForDir(Dir d) const;
};
