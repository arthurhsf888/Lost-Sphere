#pragma once
#include "Scene.h"
#include "SceneManager.h"
#include "Text.h"
#include "Boss.h"

/* -------------------------------------------------------------------------
 * Cena de introdução do chefe antes da batalha:
 * - Mostra texto narrativo por alguns segundos
 * - Ou avança imediatamente se o jogador apertar uma tecla
 * - Serve como "transição dramática" antes do combate real
 * ------------------------------------------------------------------------- */
class BossIntroScene : public Scene {
public:
    // Construtor recebe:
    // - SceneManager para trocar de cena
    // - Text para renderizar textos
    // - Ponteiro para o boss atual
    // - Nome da próxima cena (batalha)
    BossIntroScene(SceneManager& sm, Text* text, Boss* boss, const char* nextBattleSceneId)
      : sm_(sm), text_(text), boss_(boss), nextSceneId_(nextBattleSceneId) {}

    // Entrada (teclado)
    void handleEvent(const SDL_Event& e) override;

    // Atualização temporal (cronômetro de auto-avance)
    void update(float dt) override;

    // Renderização da introdução visual
    void render(SDL_Renderer* r) override;

private:
    SceneManager& sm_;
    Text* text_ = nullptr;
    Boss* boss_ = nullptr;
    const char* nextSceneId_ = nullptr;

    float t_ = 0.f;             // tempo acumulado desde que a cena iniciou
    float showSeconds_ = 1.5f;  // tempo máximo antes de avançar automaticamente
};
