#pragma once
#include "Scene.h"
#include "SceneManager.h"
#include "Text.h"

class MenuScene : public Scene {
public:
  MenuScene(SceneManager& sm, Text* text) : sm_(sm), text_(text) {}
  void handleEvent(const SDL_Event& e) override;
  void update(float) override {}
  void render(SDL_Renderer* r) override;

private:
  SceneManager& sm_;
  Text* text_;
  int selected_ = 0;
};
