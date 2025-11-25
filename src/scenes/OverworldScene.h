#pragma once
#include "Scene.h"
#include "SceneManager.h"
#include <vector>

class OverworldScene : public Scene {
public:
    explicit OverworldScene(SceneManager& sm) : sm_(sm) {}

    void handleEvent(const SDL_Event& e) override;
    void update(float dt) override;
    void render(SDL_Renderer* r) override;

private:
    SceneManager& sm_;
    SDL_FRect player_{ 380.f, 280.f, 24.f, 24.f };
    float speed_ = 160.f;
    float vx_ = 0.f, vy_ = 0.f;

    // colisões simples (paredes)
    std::vector<SDL_Rect> walls_{
        {100,100, 600,20}, {100,100, 20,400}, {100,480, 600,20}, {680,100, 20,400}
    };

    // “portal” para iniciar batalha
    SDL_Rect portal_{ 610, 410, 50, 50 };

    bool aabbIntersect(const SDL_FRect& a, const SDL_Rect& b);
    void resolveCollisions(SDL_FRect& next);
};
