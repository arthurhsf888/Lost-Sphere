#pragma once
#include "Scene.h"
#include "SceneManager.h"
#include "Text.h"
#include "GameState.h"
#include <array>
#include <string>

class SelectSetScene : public Scene {
public:
    SelectSetScene(SceneManager& sm, Text* text, GameState* gs)
      : sm_(sm), text_(text), gs_(gs) {}

    void handleEvent(const SDL_Event& e) override;
    void update(float) override {}
    void render(SDL_Renderer* r) override;

private:
    SceneManager& sm_;
    Text* text_;
    GameState* gs_;
    int idx_ = 0; // 0 guerreiro, 1 mago, 2 caçador
};
