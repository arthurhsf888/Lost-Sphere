#pragma once
#include "Scene.h"
#include "SceneManager.h"
#include "Text.h"
#include <SDL.h>

class MenuScene : public Scene {
public:
  explicit MenuScene(SceneManager& sm, Text* text = nullptr)
      : sm_(sm), text_(text) {}

  ~MenuScene() override {
    if (bg_) SDL_DestroyTexture(bg_);
  }

  void handleEvent(const SDL_Event& e) override;
  void update(float) override {}                  // menu não precisa atualizar nada por frame
  void render(SDL_Renderer* r) override;

private:
  SceneManager& sm_;
  Text* text_ = nullptr;

  int selected_ = 0;          // <— FALTAVA
  SDL_Texture* bg_ = nullptr; // <— fundo opcional
};
