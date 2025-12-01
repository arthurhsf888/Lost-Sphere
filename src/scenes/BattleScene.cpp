#include "BattleScene.h"
#include <algorithm>
#include <cmath>
#include "../SoundManager.h"
#include "BossSilencio.h"

extern SoundManager gSound;

/* Utilitário: desenha um círculo preenchido.
 * Usado para efeitos visuais de golpes. */
static void drawFilledCircle(SDL_Renderer* r, int cx, int cy, int radius) {
    if (radius <= 0) return;
    for (int dy = -radius; dy <= radius; ++dy) {
        int dxMax = int(std::sqrt(float(radius * radius - dy * dy)));
        SDL_Rect line{ cx - dxMax, cy + dy, 2*dxMax + 1, 1 };
        SDL_RenderFillRect(r, &line);
    }
}

/* ---------------------------------------------------------------------
 * Destrutor: libera todas as texturas carregadas dinamicamente.
 * ------------------------------------------------------------------ */
BattleScene::~BattleScene() {
    if (heroIdleSheet_.tex)  SDL_DestroyTexture(heroIdleSheet_.tex);
    if (heroAtk1Sheet_.tex)  SDL_DestroyTexture(heroAtk1Sheet_.tex);
    if (heroAtk2Sheet_.tex)  SDL_DestroyTexture(heroAtk2Sheet_.tex);
    if (bossSheet_.tex)      SDL_DestroyTexture(bossSheet_.tex);
    if (bg_)                 SDL_DestroyTexture(bg_);
}

/* ---------------------------------------------------------------------
 * Reinicializa a cena ao entrar em uma batalha:
 * - Reseta HP/ST
 * - Define ataques da classe
 * - Reseta animações
 * - Carrega música de batalha
 * ------------------------------------------------------------------ */
void BattleScene::resetFromSet() {
    if (gs_ && boss_) {
        if (gs_->lastPortalId.empty()) {
            std::string n = boss_->name();
            if      (n == "Furia")    gs_->lastPortalId = "furia";
            else if (n == "Tempo")    gs_->lastPortalId = "tempo";
            else if (n == "Silencio") gs_->lastPortalId = "silencio";
            else if (n == "Orgulho")  gs_->lastPortalId = "orgulho";
        }
    }

    // Reinicia turnos do boss Silencio (zera multiplicador de dano)
    if (auto* s = dynamic_cast<BossSilencio*>(boss_)) {
        s->reset();
    }

    // Carrega stats iniciais
    playerHPMax_ = gs_->hpMax;
    playerSTMax_ = gs_->stMax;

    switch (gs_->set) {
        case PlayerSet::Guerreiro:
            playerHPMax_ += 15;
            atkA_ = {"Corte Rapido", 3, 7,  DamageType::Fisico};
            atkB_ = {"Golpe Pesado", 6, 13, DamageType::Fisico};
            break;
        case PlayerSet::Mago:
            atkA_ = {"Bola de Fogo", 6, 16, DamageType::Magico};
            atkB_ = {"Raio",         5, 13, DamageType::Magico};
            break;
        case PlayerSet::Cacador:
            playerSTMax_ += 20;
            atkA_ = {"Flecha Precisa", 4, 8,  DamageType::Distancia};
            atkB_ = {"Tiro Triplo",    5, 9, DamageType::Distancia};
            break;
    }

    playerHP_ = playerHPMax_;
    playerST_ = playerSTMax_;
    boss_->hp = boss_->maxHP();

    phase_               = Phase::Menu;
    nextPhaseAfterEnemy_ = Phase::Menu;
    menuIndex_           = 0;
    atkIndex_            = 0;
    lastDamageP_         = 0;
    lastDamageE_         = 0;
    errorMsg_.clear();
    infoMsg_.clear();
    dmgMsg_.clear();

    wasEnraged_        = boss_->enraged();
    enrageFlash_       = 0.f;
    bossAnimState_     = BossAnimState::Idle;
    bossAttackTimer_   = 0.f;
    playerResultTimer_ = 0.f;

    heroDef_   = heroAnimFor(gs_->set);
    heroState_ = HeroAnimState::Idle;
    heroAnim_  = Anim{};
    vfx_       = HeroVfx{};

    if (heroIdleSheet_.tex) { SDL_DestroyTexture(heroIdleSheet_.tex); heroIdleSheet_.tex = nullptr; }
    if (heroAtk1Sheet_.tex) { SDL_DestroyTexture(heroAtk1Sheet_.tex); heroAtk1Sheet_.tex = nullptr; }
    if (heroAtk2Sheet_.tex) { SDL_DestroyTexture(heroAtk2Sheet_.tex); heroAtk2Sheet_.tex = nullptr; }
    if (bossSheet_.tex)     { SDL_DestroyTexture(bossSheet_.tex);     bossSheet_.tex     = nullptr; }

    if (gSound.ok()) gSound.playMusic("battle_theme", -1);
}


/* ---------------------------------------------------------------------
 * Entrada do jogador (menu, ataques, navegação).
 * Controla a mudança de estado da batalha.
 * ------------------------------------------------------------------ */
void BattleScene::handleEvent(const SDL_Event& e) {
    if (!initialized_) { resetFromSet(); initialized_ = true; }
    if (e.type != SDL_KEYDOWN) return;
    const SDL_Keycode key = e.key.keysym.sym;

    // ------------------- MENU PRINCIPAL ---------------------
    if (phase_ == Phase::Menu) {
        if (key == SDLK_UP)   { menuIndex_ = (menuIndex_ + 2) % 3; if (gSound.ok()) gSound.playSfx("select_button"); }
        if (key == SDLK_DOWN) { menuIndex_ = (menuIndex_ + 1) % 3; if (gSound.ok()) gSound.playSfx("select_button"); }

        if (key == SDLK_RETURN) {
            if (gSound.ok()) gSound.playSfx("click_button");
            if (menuIndex_ == 0) phase_ = Phase::ChoosingAttack;
            else {
                phase_ = Phase::PlayerTurn;
                doPlayerAction();
            }
        }

        if (key == SDLK_ESCAPE) {
            initialized_ = false;
            if (gSound.ok()) gSound.playMusic("overworld_theme", -1);
            sm_.setActive("overworld");
        }
    }

    // ------------------- ESCOLHA DE ATAQUE -------------------
    else if (phase_ == Phase::ChoosingAttack) {
        if (key == SDLK_UP || key == SDLK_DOWN) {
            atkIndex_ = 1 - atkIndex_;
            if (gSound.ok()) gSound.playSfx("select_button");
        }

        if (key == SDLK_ESCAPE) {
            if (gSound.ok()) gSound.playSfx("click_button");
            phase_ = Phase::Menu;
        }

        if (key == SDLK_RETURN) {
            if (gSound.ok()) gSound.playSfx("click_button");
            phase_ = Phase::PlayerTurn;
            doPlayerAction();
        }
    }

    // ------------------- FIM DE BATALHA ----------------------
    else if (phase_ == Phase::Win || phase_ == Phase::Lose) {
        if (key == SDLK_RETURN || key == SDLK_ESCAPE) {
            initialized_ = false;
            if (gSound.ok()) gSound.playMusic("overworld_theme", -1);
            sm_.setActive("overworld");
        }
    }
}

/* ---------------------------------------------------------------------
 * Executa a ação do jogador:
 * - Ataques (com VFX)
 * - Poções
 * - Recuperação de ST
 * - Aplica dano e verifica vitória
 * ------------------------------------------------------------------ */
void BattleScene::doPlayerAction() {
    lastDamageP_ = 0;
    lastDamageE_ = 0;

    // limpa mensagens
    errorMsg_.clear();
    infoMsg_.clear();
    dmgMsg_.clear();

    vfx_ = HeroVfx{};
    heroState_ = HeroAnimState::Idle;

    // Centros do player/boss (para efeitos visuais)
    float heroCenterX = heroX_ + heroDef_.frameW * heroScale_ * 0.5f;
    float heroCenterY = heroY_ + heroDef_.frameH * heroScale_ * 0.5f;

    SpriteInfo si = boss_->sprite();
    float bossCenterX = bossX_ + si.fw * bossScale_ * 0.5f;
    float bossCenterY = bossY_ + si.fh * bossScale_ * 0.5f;

    // ------------------ ATAQUE --------------------
    if (menuIndex_ == 0) {
        const Attack& atk = (atkIndex_ == 0 ? atkA_ : atkB_);

        if (playerST_ < atk.cost) {
            errorMsg_ = "ST Insuficiente!";
            phase_ = Phase::Menu;
            return;
        }

        playerST_ -= atk.cost;

        // cálculo de dano com multiplicador da classe
        float mult = Balance::bossFuriaMultiplier(atk.type);
        lastDamageP_ = std::max(1, int(std::round(atk.dmg * mult)));
        boss_->hp = std::max(0, boss_->hp - lastDamageP_);

        dmgMsg_ = "Dano: " + std::to_string(lastDamageP_);
        dmgMsgTimer_ = 2.0f;

        heroState_ = (atkIndex_ == 0 ? HeroAnimState::Attack1 : HeroAnimState::Attack2);
        heroAnim_  = Anim{};

        // SFX do ataque
        if (gSound.ok()) {
            switch (atk.type) {
                case DamageType::Fisico:     gSound.playSfx("sword"); break;
                case DamageType::Magico:     gSound.playSfx("spell"); break;
                case DamageType::Distancia:  gSound.playSfx("bow");   break;
            }
        }

        // Cria VFX da habilidade
        SDL_Color purple{255, 0, 0, 50};
        if (atk.type == DamageType::Fisico) {
            vfx_.type     = HeroVfxType::Slash;
            vfx_.duration = 0.25f;
            vfx_.startX   = bossCenterX;
            vfx_.startY   = bossCenterY;
            vfx_.color    = purple;
        }
        else if (atk.type == DamageType::Distancia) {
            vfx_.type     = HeroVfxType::Projectile;
            vfx_.duration = 0.4f;
            vfx_.startX   = heroCenterX + 10.f;
            vfx_.startY   = heroCenterY - 10.f;
            vfx_.endX     = bossCenterX;
            vfx_.endY     = bossCenterY - 20.f;
            vfx_.color    = purple;
        }
        else if (atk.type == DamageType::Magico) {
            vfx_.type     = HeroVfxType::Buff;
            vfx_.duration = 0.35f;
            vfx_.startX   = heroCenterX;
            vfx_.startY   = heroCenterY;
            vfx_.color    = purple;
        }

        // verifica morte do boss
        if (boss_->hp <= 0) {
            markBossDefeated();
            phase_ = Phase::Win;
            return;
        }
    }

    // ------------------ POÇÃO ----------------------
    else if (menuIndex_ == 1) {
        if (gs_->potions > 0) {
            gs_->potions--;
            playerHP_ = std::min(playerHPMax_, playerHP_ + 12);

            infoMsg_ = "Voce usou uma Poção.";
            infoMsgTimer_ = 2.0f;

            heroState_ = HeroAnimState::Potion;
            heroAnim_  = Anim{};

            if (gSound.ok()) gSound.playSfx("use_potion");

            vfx_.type     = HeroVfxType::Buff;
            vfx_.duration = 0.45f;
            vfx_.startX   = heroCenterX;
            vfx_.startY   = heroCenterY;
            vfx_.color    = SDL_Color{ 80, 220, 120, 50 };
        } else {
            errorMsg_ = "Sem pocoes!";
            phase_ = Phase::Menu;
            return;
        }
    }

    // ------------------ RECUPERAR ST ----------------------
    else if (menuIndex_ == 2) {
        playerST_ = std::min(playerSTMax_, playerST_ + 4);

        infoMsg_ = "Voce respirou e recuperou ST.";
        infoMsgTimer_ = 2.0f;

        heroState_ = HeroAnimState::Recover;
        heroAnim_  = Anim{};

        if (gSound.ok()) gSound.playSfx("recover_st");

        vfx_.type     = HeroVfxType::Buff;
        vfx_.duration = 0.45f;
        vfx_.startX   = heroCenterX;
        vfx_.startY   = heroCenterY;
        vfx_.color    = SDL_Color{ 245, 220, 120, 50 };
    }

    // Após a ação, aguarda animação
    phase_ = Phase::PlayerTurn;
    playerResultTimer_ = 2.0f;
}

/* ---------------------------------------------------------------------
 * Turno do inimigo: calcula dano, toca animação e verifica derrota.
 * ------------------------------------------------------------------ */
void BattleScene::doEnemyAction() {
    lastDamageE_ = boss_->doTurn();

    if (boss_->hp <= 0) {
        markBossDefeated();
        nextPhaseAfterEnemy_ = Phase::Win;
        return;
    }

    playerHP_ = std::max(0, playerHP_ - lastDamageE_);

    if (lastDamageE_ > 0 && gSound.ok())
        gSound.playSfx("boss_attack");

    nextPhaseAfterEnemy_ = (playerHP_ <= 0 ? Phase::Lose : Phase::Menu);
}

/* ---------------------------------------------------------------------
 * Marca no GameState o boss derrotado para liberar portais no overworld.
 * ------------------------------------------------------------------ */
void BattleScene::markBossDefeated() {
    if (!gs_) return;

    const std::string& id = gs_->lastPortalId;

    if      (id == "furia")    gs_->deadFuria    = true;
    else if (id == "tempo")    gs_->deadTempo    = true;
    else if (id == "silencio") gs_->deadSilencio = true;
    else if (id == "orgulho")  gs_->deadOrgulho  = true;
}

/* ---------------------------------------------------------------------
 * Atualização geral da cena:
 * - Avança animações
 * - Controla VFX
 * - Controla a máquina de estados da batalha
 * ------------------------------------------------------------------ */
void BattleScene::update(float dt) {
    if (!initialized_) { resetFromSet(); initialized_ = true; }

    // ---- Atualiza animação do herói ----
    int heroFrames = heroDef_.idleFrames;
    float heroFps  = 6.f;

    switch (heroState_) {
        case HeroAnimState::Idle:     heroFrames = heroDef_.idleFrames; heroFps = 6.f;  break;
        case HeroAnimState::Attack1:  heroFrames = heroDef_.atk1Frames; heroFps = 10.f; break;
        case HeroAnimState::Attack2:  heroFrames = heroDef_.atk2Frames; heroFps = 10.f; break;
        case HeroAnimState::Potion:
        case HeroAnimState::Recover:  heroFrames = heroDef_.idleFrames; heroFps = 6.f;  break;
    }

    heroAnim_.fps   = heroFps;
    heroAnim_.count = std::max(1, heroFrames);
    heroAnim_.update(dt);

    // ---- Atualiza efeitos visuais ----
    if (vfx_.type != HeroVfxType::None) {
        vfx_.t += dt;
        if (vfx_.t >= vfx_.duration) vfx_.type = HeroVfxType::None;
    }

    // ---- Resolução de turno do jogador ----
    if (phase_ == Phase::PlayerTurn) {
        if (playerResultTimer_ > 0.f) {
            playerResultTimer_ -= dt;
            if (playerResultTimer_ <= 0.f) {
                if (boss_->hp > 0) {
                    phase_ = Phase::EnemyTurn;
                    bossAnimState_ = BossAnimState::Idle;
                    bossAttackTimer_ = 0.f;
                } else {
                    phase_ = Phase::Win;
                }
            }
        }
    }

    // ---- Turno do inimigo ----
    if (phase_ == Phase::EnemyTurn) {
        if (bossAnimState_ != BossAnimState::Attacking) {
            bossAnimState_ = BossAnimState::Attacking;
            bossAttackTimer_ = bossAttackDuration_;
            doEnemyAction();
        } else {
            bossAttackTimer_ -= dt;
            if (bossAttackTimer_ <= 0.f) {
                bossAnimState_ = BossAnimState::Idle;
                phase_ = nextPhaseAfterEnemy_;

                if (phase_ == Phase::Menu && heroState_ != HeroAnimState::Idle) {
                    heroState_ = HeroAnimState::Idle;
                    heroAnim_  = Anim{};
                }
            }
        }
    }

    // ---- Atualiza animação do boss ----
    SpriteInfo si = boss_->sprite();
    if (bossAnimState_ == BossAnimState::Attacking &&
        si.attackStartIdx >= 0 && si.attackCount > 0) {
        animBoss_.fps   = 10.f;
        animBoss_.count = si.attackCount;
    } else {
        animBoss_.fps   = 4.f;
        animBoss_.count = 1;
    }
    animBoss_.update(dt);

    // ---- Controle de mudança para modo ENRAGE ----
    tBoss_ += dt;
    if (enrageFlash_ > 0.f) enrageFlash_ = std::max(0.f, enrageFlash_ - dt);

    bool enr = boss_->enraged();
    if (enr && !wasEnraged_) enrageFlash_ = 0.25f;
    wasEnraged_ = enr;

    // ---- Limpa mensagens temporárias ----
    if (dmgMsgTimer_ > 0.f) {
        dmgMsgTimer_ -= dt;
        if (dmgMsgTimer_ <= 0.f) dmgMsg_.clear();
    }
    if (infoMsgTimer_ > 0.f) {
        infoMsgTimer_ -= dt;
        if (infoMsgTimer_ <= 0.f) infoMsg_.clear();
    }
}

/* ---------------------------------------------------------------------
 * Desenha uma barra genérica (HP/ST/etc).
 * ------------------------------------------------------------------ */
void BattleScene::drawBars(SDL_Renderer* r, int x, int y, int w, int h, float ratio) {
    SDL_Rect bg{ x,y,w,h };
    SDL_SetRenderDrawColor(r, 50, 50, 60, 255);
    SDL_RenderFillRect(r, &bg);

    SDL_Rect fg{
        x+2, y+2,
        int((w-4) * std::max(0.f, std::min(1.f, ratio))),
        h-4
    };
    SDL_SetRenderDrawColor(r, 200, 220, 240, 255);
    SDL_RenderFillRect(r, &fg);

    SDL_SetRenderDrawColor(r, 20, 20, 25, 255);
    SDL_RenderDrawRect(r, &bg);
}

/* ---------------------------------------------------------------------
 * Renderização completa da cena:
 * - Fundo
 * - Barras
 * - Sprites do herói e boss
 * - Efeitos visuais
 * - HUD e menus
 * ------------------------------------------------------------------ */
void BattleScene::render(SDL_Renderer* r) {
    // ---- Fundo da cena ----
    if (!bg_) {
        std::string path = "assets/backgrounds/default.png";
        std::string n = boss_->name();
        if (n == "Furia")         path = "assets/backgrounds/furia_bg.png";
        else if (n == "Tempo")    path = "assets/backgrounds/tempo_bg.png";
        else if (n == "Silencio") path = "assets/backgrounds/silencio_bg.png";
        else if (n == "Orgulho")  path = "assets/backgrounds/orgulho_bg.png";
        bg_ = loadTexture(r, path);
    }

    if (bg_) {
        SDL_Rect full{0,0,1280,720};
        SDL_RenderCopy(r, bg_, nullptr, &full);
    } else {
        SDL_SetRenderDrawColor(r, 18,12,20,255);
        SDL_RenderClear(r);
    }

    if (!initialized_) { resetFromSet(); initialized_ = true; }

    // ---- Barras de HP/ST ----
    drawBars(r, 80, 80, 240, 16, float(playerHP_) / playerHPMax_);
    drawBars(r, 950, 80, 240, 16, float(boss_->hp)  / boss_->maxHP());

    // ---- Carrega sprites do herói se necessário ----
    if (!heroIdleSheet_.tex && !heroDef_.idlePath.empty())
        heroIdleSheet_ = loadSpriteSheet(r, heroDef_.idlePath, heroDef_.frameW, heroDef_.frameH);
    if (!heroAtk1Sheet_.tex && !heroDef_.atk1Path.empty())
        heroAtk1Sheet_ = loadSpriteSheet(r, heroDef_.atk1Path, heroDef_.frameW, heroDef_.frameH);
    if (!heroAtk2Sheet_.tex && !heroDef_.atk2Path.empty())
        heroAtk2Sheet_ = loadSpriteSheet(r, heroDef_.atk2Path, heroDef_.frameW, heroDef_.frameH);

    // ---- Escolhe sprite do herói conforme estado ----
    SpriteSheet* heroSheet = &heroIdleSheet_;
    int frameCount = std::max(1, heroDef_.idleFrames);

    switch (heroState_) {
        case HeroAnimState::Idle:
            heroSheet  = &heroIdleSheet_;
            frameCount = heroDef_.idleFrames;
            break;
        case HeroAnimState::Attack1:
            heroSheet  = &heroAtk1Sheet_;
            frameCount = heroDef_.atk1Frames;
            break;
        case HeroAnimState::Attack2:
            heroSheet  = &heroAtk2Sheet_;
            frameCount = heroDef_.atk2Frames;
            break;
        case HeroAnimState::Potion:
        case HeroAnimState::Recover:
            heroSheet  = &heroIdleSheet_;
            frameCount = heroDef_.idleFrames;
            break;
    }

    // ---- Desenha o herói ----
    int heroW = int(heroDef_.frameW * heroScale_);
    int heroH = int(heroDef_.frameH * heroScale_);
    SDL_Rect playerDst{ heroX_, heroY_, heroW, heroH };

    if (heroSheet && heroSheet->tex) {
        int frame = heroAnim_.frameOffset();
        if (frame >= frameCount) frame = frameCount - 1;
        SDL_Rect src = heroSheet->frameRect(frame);
        SDL_RenderCopy(r, heroSheet->tex, &src, &playerDst);
    } else {
        SDL_SetRenderDrawColor(r, 200, 220, 240, 255);
        SDL_RenderFillRect(r, &playerDst);
    }

    // ---- Carrega e desenha o boss ----
    if (!bossSheet_.tex) {
        SpriteInfo si0 = boss_->sprite();
        bossSheet_ = loadSpriteSheet(r, si0.path, si0.fw, si0.fh);
    }

    SpriteInfo si = boss_->sprite();
    int bossW = int(si.fw * bossScale_);
    int bossH = int(si.fh * bossScale_);
    SDL_Rect bossDst{ bossX_, bossY_, bossW, bossH };

    int idx = si.idleIdx;
    if (bossAnimState_ == BossAnimState::Attacking &&
        si.attackStartIdx >= 0 && si.attackCount > 0) {
        int offset = animBoss_.frameOffset();
        if (offset >= si.attackCount) offset = si.attackCount - 1;
        idx = si.attackStartIdx + offset;
    } else {
        idx = si.idleIdx;
        if (si.enrageIdx >= 0 && boss_->enraged())
            idx = si.enrageIdx;
    }

    if (bossSheet_.tex) {
        SDL_Rect src = bossSheet_.frameRect(idx);
        SDL_RenderCopy(r, bossSheet_.tex, &src, &bossDst);
    } else {
        SDL_SetRenderDrawColor(r, 220, 120, 120, 255);
        SDL_RenderFillRect(r, &bossDst);
    }

    // ---- Efeitos visuais do jogador ----
    if (vfx_.type != HeroVfxType::None) {
        float alpha = (vfx_.duration > 0.f)
                        ? std::clamp(vfx_.t / vfx_.duration, 0.f, 1.f)
                        : 1.f;

        SDL_SetRenderDrawColor(r, vfx_.color.r, vfx_.color.g, vfx_.color.b, vfx_.color.a);

        if (vfx_.type == HeroVfxType::Slash) {
            int radius = int(50 * (1.0f - 0.4f * alpha));
            drawFilledCircle(r, int(vfx_.startX), int(vfx_.startY), radius);
        }
        else if (vfx_.type == HeroVfxType::Projectile) {
            float t = alpha;
            float px = vfx_.startX + (vfx_.endX - vfx_.startX) * t;
            float py = vfx_.startY + (vfx_.endY - vfx_.startY) * t;
            drawFilledCircle(r, int(px), int(py), 10);
        }
        else if (vfx_.type == HeroVfxType::Buff) {
            float pulse = 1.0f + 0.3f * std::sin(alpha * 3.14159f);
            int baseR = std::max(heroW, heroH) / 3;
            int radius = int(baseR * pulse);
            float hx = float(heroX_) + heroW * 0.5f;
            float hy = float(heroY_) + heroH * 0.5f;
            drawFilledCircle(r, int(hx), int(hy), radius);
        }
    }

    // ---- HUD e textos ----
    if (text_) {
        SDL_Color white{245,245,255,255};
        SDL_Color red{255,0,0,255};
        SDL_Color yellow{202,252,0,255};

        text_->draw(r, "Chefe: " + std::string(boss_->name()) + (boss_->enraged() ? " (ENRAGE)" : ""), 1054, 104, red);
        text_->draw(r, "Set: "   + gs_->setName(), 80, 104, white);
        text_->draw(r, "HP: "    + std::to_string(playerHP_) + "/" + std::to_string(playerHPMax_), 80, 124, yellow);
        text_->draw(r, "ST: "    + std::to_string(playerST_) + "/" + std::to_string(playerSTMax_), 80, 144, yellow);
        text_->draw(r, "Pocoes: "+ std::to_string(gs_->potions), 80, 164, yellow);
    }

    // ---- Menus ----
    if (phase_ == Phase::Menu) {
        const char* labels[3] = {"Atacar","Poção HP","Recuperar ST"};
        for (int i=0;i<3;i++){
            SDL_Rect b{ 90, 390 + i*50, 200, 40 };
            if (i == menuIndex_) SDL_SetRenderDrawColor(r, 98, 0, 255, 255);
            else                 SDL_SetRenderDrawColor(r, 82, 86, 108, 255);
            SDL_RenderFillRect(r, &b);
            if (text_) {
                SDL_Color c{255,255,255,255};
                text_->draw(r, labels[i], b.x + 10, b.y + 10, c);
            }
        }
    }
    else if (phase_ == Phase::ChoosingAttack) {
        SDL_Rect b1{ 320, 390, 360, 40 };
        SDL_Rect b2{ 320, 440, 360, 40 };
        SDL_SetRenderDrawColor(r, atkIndex_==0?200:70, atkIndex_==0?200:70, atkIndex_==0?220:90, 255);
        SDL_RenderFillRect(r, &b1);
        SDL_SetRenderDrawColor(r, atkIndex_==1?200:70, atkIndex_==1?200:70, atkIndex_==1?220:90, 255);
        SDL_RenderFillRect(r, &b2);

        if (text_) {
            SDL_Color black{20,20,30,255};
            auto fmt = [](const Attack& a){
                return std::string(a.name) +
                       "  (ST " + std::to_string(a.cost) +
                       ", Dmg " + std::to_string(a.dmg) + ")";
            };
            text_->draw(r, fmt(atkA_), b1.x + 10, b1.y + 10, black);
            text_->draw(r, fmt(atkB_), b2.x + 10, b2.y + 10, black);
        }
    }
    else if (phase_ == Phase::Win || phase_ == Phase::Lose) {
        if (text_) {
            SDL_Color black{0,0,0,255};
            text_->draw(
                r,
                phase_==Phase::Win ? "Vitória! (Enter/Esc)" : "Derrota... (Enter/Esc)",
                600, 360,
                black
            );
        }
    }

    // ---- Mensagens temporárias de dano/erro ----
    if (text_) {
        if (!errorMsg_.empty()) {
            SDL_Color red{220, 60, 60, 255};
            text_->draw(r, errorMsg_, 320, 360, red);
        }

        if (!infoMsg_.empty()) {
            text_->draw(r, infoMsg_, 1280/2 + 2, 720/2 + 122, SDL_Color{0,0,0,200});
            text_->draw(r, infoMsg_, 1280/2,     720/2 + 120, SDL_Color{245,245,255,255});
        }

        if (!dmgMsg_.empty()) {
            SDL_Color yellow{255, 220, 120, 255};
            text_->draw(r, dmgMsg_, 542, 122, SDL_Color{0,0,0,200});
            text_->draw(r, dmgMsg_, 540, 120, yellow);
        }
    }

    SDL_RenderPresent(r);
}
