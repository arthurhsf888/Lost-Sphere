#include "OverworldScene.h"
#include <SDL.h>

bool OverworldScene::aabbIntersect(const SDL_FRect& a, const SDL_Rect& b) {
  return !(a.x + a.w <= b.x || a.x >= b.x + b.w ||
           a.y + a.h <= b.y || a.y >= b.y + b.h);
}

void OverworldScene::resolveCollisions(SDL_FRect& next) {
  // separação eixo a eixo (AABB simples)
  SDL_FRect test = next;
  // eixo X
  for (const auto& w : walls_) {
    if (aabbIntersect(test, w)) {
      if (vx_ > 0) test.x = w.x - test.w;
      else if (vx_ < 0) test.x = w.x + w.w;
    }
  }
  // eixo Y
  for (const auto& w : walls_) {
    if (aabbIntersect(test, w)) {
      if (vy_ > 0) test.y = w.y - test.h;
      else if (vy_ < 0) test.y = w.y + w.h;
    }
  }
  player_ = test;
}

void OverworldScene::handleEvent(const SDL_Event& e) {
  if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) {
    const bool down = (e.type == SDL_KEYDOWN);
    switch (e.key.keysym.sym) {
      case SDLK_a: case SDLK_LEFT:  vx_ = down ? -1.f : (vx_ < 0 ? 0.f : vx_); break;
      case SDLK_d: case SDLK_RIGHT: vx_ = down ?  1.f : (vx_ > 0 ? 0.f : vx_); break;
      case SDLK_w: case SDLK_UP:    vy_ = down ? -1.f : (vy_ < 0 ? 0.f : vy_); break;
      case SDLK_s: case SDLK_DOWN:  vy_ = down ?  1.f : (vy_ > 0 ? 0.f : vy_); break;
      case SDLK_ESCAPE: {
        // voltar ao menu
        sm_.setActive("menu");
      } break;
      case SDLK_e: {
        // se estiver “encostando” no portal, entra em batalha
        SDL_FRect p = player_;
        if (aabbIntersect(p, portal_)) {
          sm_.setActive("selectset");
        }
      } break;
    }
  }
}

void OverworldScene::update(float dt) {
  SDL_FRect next = player_;
  next.x += vx_ * speed_ * dt;
  next.y += vy_ * speed_ * dt;
  resolveCollisions(next);
}

void OverworldScene::render(SDL_Renderer* r) {
  // fundo
  SDL_SetRenderDrawColor(r, 12, 18, 26, 255);
  SDL_RenderClear(r);

  // paredes
  SDL_SetRenderDrawColor(r, 70, 90, 120, 255);
  for (const auto& w : walls_) SDL_RenderFillRect(r, &w);

  // portal
  SDL_SetRenderDrawColor(r, 110, 180, 255, 255);
  SDL_RenderFillRect(r, &portal_);

  // player
  SDL_Rect pr{ (int)player_.x, (int)player_.y, (int)player_.w, (int)player_.h };
  SDL_SetRenderDrawColor(r, 230, 230, 240, 255);
  SDL_RenderFillRect(r, &pr);

  SDL_RenderPresent(r);
}
