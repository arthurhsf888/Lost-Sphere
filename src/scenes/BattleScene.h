#pragma once
#include <SDL.h>
#include "Scene.h"
#include "SceneManager.h"
#include "Text.h"
#include "../GameState.h"
#include "Boss.h"
#include "Balance.h"
#include "Anim.h"
#include <array>
#include <string>
#include "Assets.h"
#include "SpriteSheet.h"
#include "../HeroAnim.h"

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
    enum class HeroAnimState { Idle, Attack1, Attack2, Potion, Recover };

    enum class HeroVfxType { None, Slash, Projectile, Buff };

    struct HeroVfx {
        HeroVfxType type = HeroVfxType::None;
        float t = 0.f;
        float duration = 0.f;
        float startX = 0.f, startY = 0.f;
        float endX = 0.f, endY = 0.f;
        SDL_Color color{255,255,255,255};
    };

    SceneManager& sm_;
    Text* text_;
    GameState* gs_;
    Boss* boss_;

    Phase phase_ = Phase::Menu;
    Phase nextPhaseAfterEnemy_ = Phase::Menu;

    int menuIndex_ = 0;
    std::array<std::string,3> menu_{ "Atacar", "Poção HP", "Recuperar ST" };

    int playerHP_ = 0, playerHPMax_ = 0;
    int playerST_ = 0, playerSTMax_ = 0;

    int atkIndex_ = 0;
    struct Attack { const char* name; int cost; int dmg; DamageType type; };
    Attack atkA_{ "Golpe A", 3, 6,  DamageType::Fisico };
    Attack atkB_{ "Golpe B", 6, 12, DamageType::Fisico };

    int lastDamageP_ = 0;
    int lastDamageE_ = 0;

    std::string errorMsg_;
    std::string infoMsg_;
    std::string dmgMsg_;

    float dmgMsgTimer_  = 0.f;
    float infoMsgTimer_ = 0.f;

    void resetFromSet();
    void doPlayerAction();
    void doEnemyAction();
    void drawBars(SDL_Renderer* r, int x, int y, int w, int h, float ratio);
    void markBossDefeated();

    bool initialized_ = false;

    SDL_Texture* bg_ = nullptr;

    SpriteSheet bossSheet_;
    Anim        animBoss_;
    float       tBoss_ = 0.f;
    bool        wasEnraged_ = false;
    float       enrageFlash_ = 0.f;

    BossAnimState bossAnimState_   = BossAnimState::Idle;
    float         bossAttackTimer_ = 0.f;
    float         bossAttackDuration_ = 0.5f;

    HeroAnimDef heroDef_;
    SpriteSheet heroIdleSheet_;
    SpriteSheet heroAtk1Sheet_;
    SpriteSheet heroAtk2Sheet_;
    Anim        heroAnim_;
    HeroAnimState heroState_ = HeroAnimState::Idle;

    float playerResultTimer_ = 0.f;

    HeroVfx vfx_;

    int heroX_ = 300;
    int heroY_ = 280;
    int bossX_ = 760;
    int bossY_ = 220;

    float heroScale_ = 2.0f;
    float bossScale_ = 6.0f;
};
