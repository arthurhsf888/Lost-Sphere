#pragma once
#include "Scene.h"
#include "SceneManager.h"
#include "Text.h"
#include "GameState.h"
#include "Boss.h"
#include "Balance.h"
#include <array>
#include <string>

class BattleScene : public Scene {
public:
    BattleScene(SceneManager& sm, Text* text, GameState* gs, Boss* boss)
      : sm_(sm), text_(text), gs_(gs), boss_(boss) {}

    void handleEvent(const SDL_Event& e) override;
    void update(float dt) override;
    void render(SDL_Renderer* r) override;

private:
    enum class Phase { Menu, ChoosingAttack, PlayerTurn, EnemyTurn, Win, Lose };
    SceneManager& sm_;
    Text* text_;
    GameState* gs_;
    Boss* boss_; // não-ownership (instância global simples)

    Phase phase_ = Phase::Menu;
    int menuIndex_ = 0;
    std::array<std::string,3> menu_{ "Atacar", "Pocao", "Desviar" };

    // atributos dependentes do set (inicializados no reset)
    int playerHP_ = 0, playerHPMax_ = 0;
    int playerST_ = 0, playerSTMax_ = 0;

    // sub menu de ataque (2 golpes)
    int atkIndex_ = 0; // 0 ou 1
    struct Attack { const char* name; int cost; int dmg; DamageType type; };
    Attack atkA_{ "Golpe A", 3, 6, DamageType::Fisico };
    Attack atkB_{ "Golpe B", 6, 12, DamageType::Fisico };

    int lastDamageP_ = 0;
    int lastDamageE_ = 0;
    std::string lastMsg_;

    void resetFromSet();       // define stats/ataques conforme gs_->set
    void doPlayerAction();     // executa a ação escolhida
    void doEnemyAction();
    void drawBars(SDL_Renderer* r, int x, int y, int w, int h, float ratio);

    bool initialized_ = false;
};
