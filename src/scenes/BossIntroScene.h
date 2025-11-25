#pragma once
#include "Scene.h"
#include "SceneManager.h"
#include "Text.h"
#include "Boss.h"

class BossIntroScene : public Scene {
public:
    BossIntroScene(SceneManager& sm, Text* text, Boss* boss)
      : sm_(sm), text_(text), boss_(boss) {}

    void handleEvent(const SDL_Event& e) override;
    void update(float) override {}
    void render(SDL_Renderer* r) override;

private:
    SceneManager& sm_;
    Text* text_;
    Boss* boss_;
};
