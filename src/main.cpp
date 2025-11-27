#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>
#include <filesystem>

#include "SceneManager.h"
#include "scenes/MenuScene.h"
#include "scenes/OverworldScene.h"
#include "scenes/BattleScene.h"
#include "scenes/SelectSetScene.h"
// #include "scenes/BossIntroScene.h"  // opcional, só use se for registrar intros
#include "Text.h"
#include "GameState.h"
#include "Assets.h"

#include "BossFuria.h"
#include "BossTempo.h"
#include "BossSilencio.h"
#include "BossOrgulho.h"

// Resolve caminhos "assets/..." a partir do executável/raiz do projeto
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

  // 2) diretório de trabalho atual
  cand.push_back(rel);

  for (auto& p : cand) {
    if (std::filesystem::exists(p)) return p;
  }
  return rel; // deixa falhar no init e mostrar caminho
}

int main(int, char**) {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
    std::cerr << "SDL_Init erro: " << SDL_GetError() << "\n";
    return 1;
  }

  // SDL_image
  int imgFlags = IMG_INIT_PNG;
  if (!(IMG_Init(imgFlags) & imgFlags)) {
    std::cerr << "IMG_Init erro: " << IMG_GetError() << "\n";
    // segue sem PNG (fallback em retângulos)
  }

  // SDL_ttf
  if (TTF_Init() != 0) {
    std::cerr << "TTF_Init erro: " << TTF_GetError() << "\n";
    // ainda dá pra rodar sem texto, mas ideal é ter TTF funcionando
  }

  SDL_Window* window = SDL_CreateWindow(
      "Lost Sphere (skeleton)",
      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      1280, 720, SDL_WINDOW_SHOWN);
  if (!window) {
    std::cerr << "SDL_CreateWindow erro: " << SDL_GetError() << "\n";
    return 1;
  }

  SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (!renderer) {
    std::cerr << "SDL_CreateRenderer erro: " << SDL_GetError() << "\n";
    return 1;
  }

  SDL_RenderSetLogicalSize(renderer, 1280, 720);  // “resolução virtual”

  Text uiText;
  uiText.init(resolveAsset("assets/fonts/DejaVuSans.ttf"), 18);

  GameState gs;

  // ---- Bosses (4) ----
  BossFuria    bossFuria;
  BossTempo    bossTempo;
  BossSilencio bossSilencio;
  BossOrgulho  bossOrgulho;

  // ---- Cenas ----
  SceneManager sm;
  sm.registerScene("menu",       std::make_unique<MenuScene>(sm, &uiText));
  sm.registerScene("overworld",  std::make_unique<OverworldScene>(sm, &uiText));
  sm.registerScene("selectset",  std::make_unique<SelectSetScene>(sm, &uiText, &gs));

  // batalhas específicas para cada portal
  sm.registerScene("battle_furia",    std::make_unique<BattleScene>(sm, &uiText, &gs, &bossFuria));
  sm.registerScene("battle_tempo",    std::make_unique<BattleScene>(sm, &uiText, &gs, &bossTempo));
  sm.registerScene("battle_silencio", std::make_unique<BattleScene>(sm, &uiText, &gs, &bossSilencio));
  sm.registerScene("battle_orgulho",  std::make_unique<BattleScene>(sm, &uiText, &gs, &bossOrgulho));

  // Se quiser usar cenas de introdução:
  // sm.registerScene("intro_furia",    std::make_unique<BossIntroScene>(sm, &uiText, &bossFuria,    "battle_furia"));
  // sm.registerScene("intro_tempo",    std::make_unique<BossIntroScene>(sm, &uiText, &bossTempo,    "battle_tempo"));
  // sm.registerScene("intro_silencio", std::make_unique<BossIntroScene>(sm, &uiText, &bossSilencio, "battle_silencio"));
  // sm.registerScene("intro_orgulho",  std::make_unique<BossIntroScene>(sm, &uiText, &bossOrgulho,  "battle_orgulho"));

  sm.setActive("menu");

  // ---- Loop principal ----
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

    SDL_Delay(1); // capping simples
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  TTF_Quit();
  IMG_Quit();
  SDL_Quit();
  return 0;
}
