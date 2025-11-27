#include "OverworldScene.h"
#include <SDL.h>
#include <string>   // para std::string nos rótulos

OverworldScene::~OverworldScene() {
  if (playerSheet_.tex) SDL_DestroyTexture(playerSheet_.tex);
  if (texPortal_)       SDL_DestroyTexture(texPortal_);
}

bool OverworldScene::aabbIntersect(const SDL_FRect& a, const SDL_Rect& b) {
  return !(a.x + a.w <= b.x || a.x >= b.x + b.w ||
           a.y + a.h <= b.y || a.y >= b.y + b.h);
}

void OverworldScene::resolveCollisions(SDL_FRect& next) {
  SDL_FRect test = next;

  // eixo X
  for (const auto& w : walls_) {
    if (aabbIntersect(test, w)) {
      if (vx_ > 0)      test.x = w.x - test.w;
      else if (vx_ < 0) test.x = w.x + w.w;
    }
  }
  // eixo Y
  for (const auto& w : walls_) {
    if (aabbIntersect(test, w)) {
      if (vy_ > 0)      test.y = w.y - test.h;
      else if (vy_ < 0) test.y = w.y + w.h;
    }
  }
  player_ = test;
}

void OverworldScene::handleEvent(const SDL_Event& e) {
  if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) {
    const bool down = (e.type == SDL_KEYDOWN);
    switch (e.key.keysym.sym) {
      case SDLK_a: case SDLK_LEFT:
        vx_ = down ? -1.f : (vx_ < 0 ? 0.f : vx_);
        if (down) facing_ = Dir::Left;
        break;
      case SDLK_d: case SDLK_RIGHT:
        vx_ = down ?  1.f : (vx_ > 0 ? 0.f : vx_);
        if (down) facing_ = Dir::Right;
        break;
      case SDLK_w: case SDLK_UP:
        vy_ = down ? -1.f : (vy_ < 0 ? 0.f : vy_);
        if (down) facing_ = Dir::Up;
        break;
      case SDLK_s: case SDLK_DOWN:
        vy_ = down ?  1.f : (vy_ > 0 ? 0.f : vy_);
        if (down) facing_ = Dir::Down;
        break;

      case SDLK_ESCAPE:
        sm_.setActive("menu");
        break;

      case SDLK_e: {
        // Interação: se encostando em algum portal, entra na batalha correspondente
        for (const auto& p : portals_) {
          if (aabbIntersect(player_, p.rect)) {
            sm_.setActive(p.battleSceneId);
            break;
          }
        }
      } break;
    }
  }
}

int OverworldScene::baseIndexForDir(Dir d) const {
  // sheet: 3 colunas × 4 linhas (Up,Left,Down,Right)
  const int cols = playerSheet_.cols;
  int row = 2; // default Down
  switch (d) {
    case Dir::Up:    row = 0; break;
    case Dir::Left:  row = 1; break;
    case Dir::Down:  row = 2; break;
    case Dir::Right: row = 3; break;
  }
  return row * cols; // primeiro índice da linha
}

void OverworldScene::update(float dt) {
  SDL_FRect next = player_;
  next.x += vx_ * speed_ * dt;
  next.y += vy_ * speed_ * dt;
  resolveCollisions(next);

  moving_ = (vx_ != 0.f || vy_ != 0.f);

  anim_.fps = 8.f;   // velocidade da caminhada
  anim_.update(dt);
}

void OverworldScene::render(SDL_Renderer* r) {
  // fundo
  SDL_SetRenderDrawColor(r, 12, 18, 26, 255);
  SDL_RenderClear(r);

  if (!mapLoaded_) {
    // ajuste os caminhos/tamanhos conforme seu assets
    // tileset 32x32; supondo 12 colunas no tileset (exemplo)
    mapLoaded_ = map_.load(r,
        "assets/tiles/tileset_dungeon.png",   // seu tileset.png
        "assets/tiles/LostSphere.csv",    // export do Tiled
        32, 32,
        12);
  }

  if (mapLoaded_) {
    map_.render(r);
  } else {
    // fallback de cor sólida caso falhe
    SDL_SetRenderDrawColor(r, 18, 22, 28, 255);
    SDL_RenderClear(r);
  }

  // paredes (moldura)
  SDL_SetRenderDrawColor(r, 70, 90, 120, 255);
  for (const auto& w : walls_) SDL_RenderFillRect(r, &w);

  // carregar textura do portal (uma vez)
  if (!texPortal_) {
    texPortal_ = loadTexture(r, "assets/sprites/fx/portal.png");
  }

  // carregar sheet do player (uma vez)
  if (!playerSheet_.tex) {
    const int FRAME_W = 32;
    const int FRAME_H = 32;
    playerSheet_ = loadSpriteSheet(r, "assets/sprites/player/overworld.png", FRAME_W, FRAME_H);
    if (playerSheet_.tex) {
      player_.w = (float)FRAME_W;
      player_.h = (float)FRAME_H;
    }
  }

  // desenhar portais (4 cantos) + rótulos
  for (const auto& p : portals_) {
    SDL_Rect dst = p.rect;
    if (texPortal_) SDL_RenderCopy(r, texPortal_, nullptr, &dst);
    else {
      SDL_SetRenderDrawColor(r, 110, 180, 255, 255);
      SDL_RenderFillRect(r, &dst);
    }

    // rótulo centralizado (se houver Text)
    if (text_) {
      int tx = dst.x + dst.w/2 - 6;
      int ty = dst.y + dst.h/2 - 10;
      std::string s(1, p.label);
      text_->draw(r, s, tx, ty);
    }
  }

  // desenhar player (sprite sheet com fallback)
  SDL_Rect pr{ (int)player_.x, (int)player_.y, (int)player_.w, (int)player_.h };

  if (playerSheet_.tex && playerSheet_.cols >= 3 && playerSheet_.rows >= 4) {
    // 3 frames por direção: parado = coluna 1; andando = 0..2
    const int base = baseIndexForDir(facing_);
    int frameCol = 1;   // idle
    int count    = 1;

    if (moving_) {
      frameCol = anim_.frameOffset() % 3; // 0..2
      count    = 3;
    }

    Anim tmp = anim_;   // só para ajustar count quando andando (se usar)
    tmp.count = count;

    const int idx = base + frameCol;
    SDL_Rect src = playerSheet_.frameRect(idx);
    SDL_RenderCopy(r, playerSheet_.tex, &src, &pr);
  } else {
    // fallback: retângulo
    SDL_SetRenderDrawColor(r, 230, 230, 240, 255);
    SDL_RenderFillRect(r, &pr);
  }

  SDL_RenderPresent(r);
}
