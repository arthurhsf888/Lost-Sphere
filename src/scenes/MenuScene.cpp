#include "MenuScene.h"
#include <SDL.h>
#include <array>
#include <string>

void MenuScene::handleEvent(const SDL_Event& e) {
  if (e.type == SDL_KEYDOWN) {
    if (e.key.keysym.sym == SDLK_UP)    selected_ = (selected_ + 2) % 3;
    if (e.key.keysym.sym == SDLK_DOWN)  selected_ = (selected_ + 1) % 3;
    if (e.key.keysym.sym == SDLK_RETURN) {
      if (selected_ == 0) sm_.setActive("overworld");
      else if (selected_ == 1) sm_.setActive("battle");
      else if (selected_ == 2) {
        SDL_Event quit{}; quit.type = SDL_QUIT; SDL_PushEvent(&quit);
      }
    }
  }
}

void MenuScene::render(SDL_Renderer* r) {
  SDL_SetRenderDrawColor(r, 15, 15, 25, 255);
  SDL_RenderClear(r);

  std::array<SDL_Rect,3> items{ SDL_Rect{280,180,240,40}, SDL_Rect{280,240,240,40}, SDL_Rect{280,300,240,40} };
  const std::array<std::string,3> labels{ "Overworld", "Batalha", "Sair" };

  for (int i=0;i<3;i++){
    if(i==selected_) SDL_SetRenderDrawColor(r, 200, 200, 220, 255);
    else             SDL_SetRenderDrawColor(r, 70, 70, 90, 255);
    SDL_RenderFillRect(r, &items[i]);

    // texto centralizado
    if (text_) {
      int tx = items[i].x + 16;
      int ty = items[i].y + 10;
      text_->draw(r, labels[i], tx, ty);
    }
  }

  SDL_RenderPresent(r);
}
