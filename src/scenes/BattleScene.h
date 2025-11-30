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
    enum class BossAnimState { Idle, Attacking };

    SceneManager& sm_;
    Text* text_;
    GameState* gs_;
    Boss* boss_; // não-ownership

    Phase phase_ = Phase::Menu;
    Phase nextPhaseAfterEnemy_ = Phase::Menu;

    int menuIndex_ = 0;
    std::array<std::string,3> menu_{ "Atacar", "Pocao", "Recuperar" };

    // atributos dependentes do set
    int playerHP_ = 0, playerHPMax_ = 0;
    int playerST_ = 0, playerSTMax_ = 0;

    // sub menu de ataque (2 golpes)
    int atkIndex_ = 0; // 0 ou 1
    struct Attack { const char* name; int cost; int dmg; DamageType type; };
    Attack atkA_{ "Golpe A", 3, 6,  DamageType::Fisico };
    Attack atkB_{ "Golpe B", 6, 12, DamageType::Fisico };

    int lastDamageP_ = 0;   // dano causado pelo player no turno
    int lastDamageE_ = 0;   // dano causado pelo chefe no turno

    // Mensagens separadas por tipo
    std::string errorMsg_;  // ST insuficiente, sem poções, etc (vermelho)
    std::string infoMsg_;   // usou poção, recuperou ST, etc (branco)
    std::string dmgMsg_;    // feedback de dano (amarelo, perto do boss)

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
    Anim         animBoss_;
    float        tBoss_ = 0.f;
    bool         wasEnraged_ = false;
    float        enrageFlash_ = 0.f;

    // Animação de ataque do boss
    BossAnimState bossAnimState_   = BossAnimState::Idle;
    float         bossAttackTimer_ = 0.f;
    float         bossAttackDuration_ = 0.5f; // duração da animação de ataque

    // Timer para mostrar resultado do player antes do boss agir
    float playerResultTimer_ = 0.f;  // em segundos
};
