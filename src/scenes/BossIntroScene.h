#pragma once
#include "Scene.h"
#include "SceneManager.h"
#include "Text.h"
#include "Boss.h"

class BossIntroScene : public Scene {
public:
    BossIntroScene(SceneManager& sm, Text* text, Boss* boss, const char* nextBattleSceneId)
      : sm_(sm), text_(text), boss_(boss), nextSceneId_(nextBattleSceneId) {}

    void handleEvent(const SDL_Event& e) override;
    void update(float dt) override;
    void render(SDL_Renderer* r) override;

private:
    SceneManager& sm_;
    Text* text_ = nullptr;
    Boss* boss_ = nullptr;
    const char* nextSceneId_ = nullptr;

    float t_ = 0.f;           // cronômetro simples
    float showSeconds_ = 1.5f; // duração antes de auto-avançar
};
