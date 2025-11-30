#pragma once
#include "Scene.h"
#include "SceneManager.h"
#include "Text.h"
#include "../GameState.h"
#include <SDL.h>
#include <array>

class SelectSetScene : public Scene {
public:
    SelectSetScene(SceneManager& sm, Text* text, GameState* gs)
        : sm_(sm), text_(text), gs_(gs) {}

    void handleEvent(const SDL_Event& e) override;
    void update(float) override {}
    void render(SDL_Renderer* r) override;

private:
    SceneManager& sm_;
    Text* text_ = nullptr;
    GameState* gs_ = nullptr;

    int idx_ = 0; // 0=Guerreiro, 1=Mago, 2=Caçador
};
