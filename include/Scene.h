#pragma once
#include <SDL.h>

class Scene {
public:
  virtual ~Scene() = default;
  virtual void handleEvent(const SDL_Event& e) = 0;
  virtual void update(float dt) = 0;
  virtual void render(SDL_Renderer* r) = 0;
};
