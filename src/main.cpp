#include <SDL.h>
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include "SceneManager.h"
#include "scenes/MenuScene.h"
#include "scenes/BattleScene.h"
#include "scenes/OverworldScene.h"
#include "Text.h"
#include "GameState.h"
#include "scenes/SelectSetScene.h"
#include "BossFuria.h"
#include "scenes/BossIntroScene.h"


// Resolve caminhos do tipo "assets/..." a partir do executável e da raiz do projeto
static std::string resolveAsset(const std::string& rel) {
  std::vector<std::string> cand;

  // 1) relativo ao diretório do executável (SDL base path)
  char* baseC = SDL_GetBasePath();
  std::string base = baseC ? baseC : "";
  if (baseC) SDL_free(baseC);
  if (!base.empty()) {
    cand.push_back(base + rel);
    cand.push_back(base + "../" + rel);
    cand.push_back(base + "../../" + rel);
  }

  // 2) relativo ao diretório de trabalho atual (por segurança)
  cand.push_back(rel);

  for (auto& p : cand) {
    if (std::filesystem::exists(p)) return p;
  }
  // último recurso: retorna rel (vai falhar no init e imprimir o caminho)
  return rel;
}

int main(int, char**) {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
    std::cerr << "SDL_Init erro: " << SDL_GetError() << "\n";
    return 1;
  }

  SDL_Window* window = SDL_CreateWindow(
      "Lost Sphere (skeleton)",
      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      800, 600, SDL_WINDOW_SHOWN);
  if (!window) { std::cerr << "SDL_CreateWindow erro: " << SDL_GetError() << "\n"; return 1; }

  SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (!renderer) { std::cerr << "SDL_CreateRenderer erro: " << SDL_GetError() << "\n"; return 1; }

  Text uiText;
  uiText.init(resolveAsset("assets/fonts/DejaVuSans.ttf"), 18);

  GameState gs;
  BossFuria bossFuria;

  SceneManager sm;
  sm.registerScene("menu",       std::make_unique<MenuScene>(sm, &uiText));
  sm.registerScene("overworld",  std::make_unique<OverworldScene>(sm));
  sm.registerScene("selectset",  std::make_unique<SelectSetScene>(sm, &uiText, &gs));
  sm.registerScene("boss_intro", std::make_unique<BossIntroScene>(sm, &uiText, &bossFuria));
  sm.registerScene("battle",     std::make_unique<BattleScene>(sm, &uiText, &gs, &bossFuria));
  sm.setActive("menu");

  bool running = true;
  Uint64 prev = SDL_GetPerformanceCounter();

  while (running) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) running = false;
      if (sm.active()) sm.active()->handleEvent(e);
    }

    Uint64 now = SDL_GetPerformanceCounter();
    float dt = float(now - prev) / SDL_GetPerformanceFrequency();
    prev = now;

    if (sm.active()) sm.active()->update(dt);
    if (sm.active()) sm.active()->render(renderer);

    // capping simples (opcional)
    SDL_Delay(1);
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  TTF_Quit();      // <— encerra TTF
  SDL_Quit();
  return 0;
}
