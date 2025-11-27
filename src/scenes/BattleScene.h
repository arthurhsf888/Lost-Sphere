#pragma once
#include "Scene.h"
#include "SceneManager.h"
#include "Text.h"
#include "GameState.h"
#include "Boss.h"
#include "Balance.h"
#include "Anim.h"
#include <array>
#include <string>
#include "Assets.h"
#include "SpriteSheet.h"

class BattleScene : public Scene {
public:
    BattleScene(SceneManager& sm, Text* text, GameState* gs, Boss* boss)
      : sm_(sm), text_(text), gs_(gs), boss_(boss) {}
    ~BattleScene() override;

    void handleEvent(const SDL_Event& e) override;
    void update(float dt) override;
    void render(SDL_Renderer* r) override;

private:
    enum class Phase { Menu, ChoosingAttack, PlayerTurn, EnemyTurn, Win, Lose };

    SceneManager& sm_;
    Text* text_;
    GameState* gs_;
    Boss* boss_; // não-ownership

    Phase phase_ = Phase::Menu;
    int menuIndex_ = 0;
    std::array<std::string,3> menu_{ "Atacar", "Pocao", "Desviar" };

    // atributos dependentes do set
    int playerHP_ = 0, playerHPMax_ = 0;
    int playerST_ = 0, playerSTMax_ = 0;

    // sub menu de ataque (2 golpes)
    int atkIndex_ = 0; // 0 ou 1
    struct Attack { const char* name; int cost; int dmg; DamageType type; };
    Attack atkA_{ "Golpe A", 3, 6,  DamageType::Fisico };
    Attack atkB_{ "Golpe B", 6, 12, DamageType::Fisico };

    int lastDamageP_ = 0;
    int lastDamageE_ = 0;
    std::string lastMsg_;

    void resetFromSet();
    void doPlayerAction();
    void doEnemyAction();
    void drawBars(SDL_Renderer* r, int x, int y, int w, int h, float ratio);

    bool initialized_ = false;

    // Background da batalha
    SDL_Texture* bg_ = nullptr;

    // Sprites
    SDL_Texture* texPlayer_ = nullptr; // player simples
    SpriteSheet  bossSheet_;
    Anim  animBoss_;        // se quiser animar boss
    float tBoss_ = 0.f;
    bool  wasEnraged_ = false;
    float enrageFlash_ = 0.f;
};
