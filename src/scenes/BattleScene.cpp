#include "BattleScene.h"
#include <algorithm>
#include <cmath>
#include "../SoundManager.h"

// instância global (definida em outro lugar, ex: main.cpp)
extern SoundManager gSound;

// Helper: desenha um círculo preenchido no SDL_Renderer
static void drawFilledCircle(SDL_Renderer* r, int cx, int cy, int radius) {
    if (radius <= 0) return;
    for (int dy = -radius; dy <= radius; ++dy) {
        int dxMax = (int)std::sqrt(float(radius * radius - dy * dy));
        SDL_Rect line{ cx - dxMax, cy + dy, 2*dxMax + 1, 1 };
        SDL_RenderFillRect(r, &line);
    }
}

BattleScene::~BattleScene() {
    // Hero
    if (heroIdleSheet_.tex)  SDL_DestroyTexture(heroIdleSheet_.tex);
    if (heroAtk1Sheet_.tex)  SDL_DestroyTexture(heroAtk1Sheet_.tex);
    if (heroAtk2Sheet_.tex)  SDL_DestroyTexture(heroAtk2Sheet_.tex);

    // Boss
    if (bossSheet_.tex)      SDL_DestroyTexture(bossSheet_.tex);

    // Background
    if (bg_)                 SDL_DestroyTexture(bg_);
}

void BattleScene::resetFromSet() {
    // stats base
    playerHPMax_ = gs_->hpMax;
    playerSTMax_ = gs_->stMax;

    // variações por set + tipos de golpe
    switch (gs_->set) {
        case PlayerSet::Guerreiro:
            playerHPMax_ += 10;
            atkA_ = {"Corte Rapido", 3, 6,  DamageType::Fisico};
            atkB_ = {"Golpe Pesado", 6, 12, DamageType::Fisico};
            break;
        case PlayerSet::Mago:
            atkA_ = {"Bola de Fogo", 5, 10, DamageType::Magico};
            atkB_ = {"Raio",         4,  8, DamageType::Magico};
            break;
        case PlayerSet::Cacador:
            playerSTMax_ += 4;
            atkA_ = {"Flecha Precisa", 4, 7,  DamageType::Distancia};
            atkB_ = {"Tiro Triplo",    6, 11, DamageType::Distancia};
            break;
    }

    playerHP_ = playerHPMax_;
    playerST_ = playerSTMax_;

    // inicializa boss
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

    // --- Hero settings ---
    heroDef_   = heroAnimFor(gs_->set);
    heroState_ = HeroAnimState::Idle;
    heroAnim_  = Anim{}; // zera tempo

    // VFX reset
    vfx_ = HeroVfx{};

    // se mudar de set no meio da mesma cena, limpamos sprites antigos
    if (heroIdleSheet_.tex) { SDL_DestroyTexture(heroIdleSheet_.tex); heroIdleSheet_.tex = nullptr; }
    if (heroAtk1Sheet_.tex) { SDL_DestroyTexture(heroAtk1Sheet_.tex); heroAtk1Sheet_.tex = nullptr; }
    if (heroAtk2Sheet_.tex) { SDL_DestroyTexture(heroAtk2Sheet_.tex); heroAtk2Sheet_.tex = nullptr; }

    // Boss sprites reset também (caso mude portal)
    if (bossSheet_.tex) { SDL_DestroyTexture(bossSheet_.tex); bossSheet_.tex = nullptr; }

    // Música de batalha
    if (gSound.ok()) {
        gSound.playMusic("battle_theme", -1);
    }
}

void BattleScene::handleEvent(const SDL_Event& e) {
    if (!initialized_) { resetFromSet(); initialized_ = true; }

    if (e.type != SDL_KEYDOWN) return;

    const SDL_Keycode key = e.key.keysym.sym;

    // ---------------- FASE: MENU PRINCIPAL ----------------
    if (phase_ == Phase::Menu) {
        if (key == SDLK_UP) {
            menuIndex_ = (menuIndex_ + 2) % 3;
            if (gSound.ok()) gSound.playSfx("select_button");
        }
        if (key == SDLK_DOWN) {
            menuIndex_ = (menuIndex_ + 1) % 3;
            if (gSound.ok()) gSound.playSfx("select_button");
        }

        if (key == SDLK_RETURN) {
            if (gSound.ok()) gSound.playSfx("click_button");

            if (menuIndex_ == 0) {
                // vai para submenu de ataques
                phase_ = Phase::ChoosingAttack;
            }
            else if (menuIndex_ == 1 || menuIndex_ == 2) {
                // Pocao ou Recuperar: aplica ação do player
                phase_ = Phase::PlayerTurn;
                doPlayerAction();   // depois dessa função ficamos em PlayerTurn, Win ou Menu (em caso de erro)
            }
        }

        if (key == SDLK_ESCAPE) {
            initialized_ = false;
            // volta pro overworld e retoma música de overworld
            if (gSound.ok()) {
                gSound.playMusic("overworld_theme", -1);
            }
            sm_.setActive("overworld");
        }
    }
    // ---------------- FASE: ESCOLHER ATAQUE ----------------
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
            doPlayerAction();   // depois dessa função ficamos em PlayerTurn, Win ou Menu (erro ST)
        }
    }
    // ---------------- FASE: RESULTADO FINAL ----------------
    else if (phase_ == Phase::Win || phase_ == Phase::Lose) {
        if (key == SDLK_RETURN || key == SDLK_ESCAPE) {
            initialized_ = false;
            // volta pro overworld e retoma música de overworld
            if (gSound.ok()) {
                gSound.playMusic("overworld_theme", -1);
            }
            sm_.setActive("overworld");
        }
    }

    // OBS: fase PlayerTurn agora é automática (timer), então
    // não tratamos teclas nela.
}

void BattleScene::doPlayerAction() {
    lastDamageP_ = 0;
    lastDamageE_ = 0;

    // limpa mensagens do turno anterior
    errorMsg_.clear();
    infoMsg_.clear();
    dmgMsg_.clear();

    // limpa VFX antigo
    vfx_ = HeroVfx{};

    // estado padrão do herói: Idle (vai ser sobrescrito abaixo se for o caso)
    heroState_ = HeroAnimState::Idle;

    // Coordenadas "centro" do herói e do boss (para VFX) considerando escala
    float heroCenterX = float(heroX_) + heroDef_.frameW * heroScale_ * 0.5f;
    float heroCenterY = float(heroY_) + heroDef_.frameH * heroScale_ * 0.5f;

    SpriteInfo siForPos = boss_->sprite();
    float bossCenterX = float(bossX_) + siForPos.fw * bossScale_ * 0.5f;
    float bossCenterY = float(bossY_) + siForPos.fh * bossScale_ * 0.5f;

    // --- Ação "Atacar" --- (menuIndex_ == 0)
    if (menuIndex_ == 0) {
        const Attack& atk = (atkIndex_ == 0 ? atkA_ : atkB_);

        // Sem ST suficiente → não passa turno, volta pro menu
        if (playerST_ < atk.cost) {
            errorMsg_ = "ST Insuficiente!";
            phase_ = Phase::Menu;
            return;
        }

        // Tem ST suficiente → executa ataque normalmente
        playerST_ -= atk.cost;

        float mult = Balance::bossFuriaMultiplier(atk.type);
        lastDamageP_ = std::max(1, int(std::round(atk.dmg * mult)));
        boss_->hp = std::max(0, boss_->hp - lastDamageP_);

        // feedback principal de dano
        dmgMsg_ = "Dano: " + std::to_string(lastDamageP_);

        // Animação do herói: Attack1/Attack2
        heroState_ = (atkIndex_ == 0 ? HeroAnimState::Attack1 : HeroAnimState::Attack2);
        heroAnim_  = Anim{}; // reseta ciclo de animação

        // Som do golpe, dependendo do tipo
        if (gSound.ok()) {
            switch (atk.type) {
                case DamageType::Fisico:     gSound.playSfx("sword"); break;
                case DamageType::Magico:     gSound.playSfx("spell"); break;
                case DamageType::Distancia:  gSound.playSfx("bow");   break;
            }
        }

        // Cor roxa para qualquer ataque
        SDL_Color purple{255, 0, 0, 50};

        // ----- VFX dependendo do tipo de dano -----
        if (atk.type == DamageType::Fisico) {
            // Slash roxo perto do boss
            vfx_.type     = HeroVfxType::Slash;
            vfx_.duration = 0.25f;
            vfx_.t        = 0.f;
            vfx_.startX   = bossCenterX;
            vfx_.startY   = bossCenterY;
            vfx_.endX     = bossCenterX;
            vfx_.endY     = bossCenterY;
            vfx_.color    = purple;
        }
        else if (atk.type == DamageType::Distancia) {
            // Projétil roxo do herói até o boss
            vfx_.type     = HeroVfxType::Projectile;
            vfx_.duration = 0.4f;
            vfx_.t        = 0.f;
            vfx_.startX   = heroCenterX + 10.f;
            vfx_.startY   = heroCenterY - 10.f;
            vfx_.endX     = bossCenterX;
            vfx_.endY     = bossCenterY - 20.f;
            vfx_.color    = purple;
        }
        else if (atk.type == DamageType::Magico) {
            // Aura roxa mágica ao redor do herói
            vfx_.type     = HeroVfxType::Buff;
            vfx_.duration = 0.35f;
            vfx_.t        = 0.f;
            vfx_.startX   = heroCenterX;
            vfx_.startY   = heroCenterY;
            vfx_.endX     = heroCenterX;
            vfx_.endY     = heroCenterY;
            vfx_.color    = purple;
        }

        // Chefe morreu -> termina batalha E marca como derrotado no GameState
        if (boss_->hp <= 0) {
            markBossDefeated();
            phase_ = Phase::Win;
            return;
        }
    }
    // --- Ação "Pocao" --- (menuIndex_ == 1)
    else if (menuIndex_ == 1) {
        if (gs_->potions > 0) {
            gs_->potions--;
            playerHP_ = std::min(playerHPMax_, playerHP_ + 12);
            infoMsg_ = "Voce usou uma Pocao.";
            heroState_ = HeroAnimState::Potion;
            heroAnim_  = Anim{};

            // SFX de usar poção
            if (gSound.ok()) {
                gSound.playSfx("use_potion");
            }

            // Aura verde de cura em volta do herói
            vfx_.type     = HeroVfxType::Buff;
            vfx_.duration = 0.45f;
            vfx_.t        = 0.f;
            vfx_.startX   = heroCenterX;
            vfx_.startY   = heroCenterY;
            vfx_.endX     = heroCenterX;
            vfx_.endY     = heroCenterY;
            vfx_.color    = SDL_Color{ 80, 220, 120, 50 }; // verde
        } else {
            errorMsg_ = "Sem pocoes!";
            phase_ = Phase::Menu;
            return;
        }
    }
    // --- Ação "Recuperar" --- (menuIndex_ == 2)
    else if (menuIndex_ == 2) {
        playerST_ = std::min(playerSTMax_, playerST_ + 4);
        infoMsg_ = "Voce respirou e recuperou ST.";
        heroState_ = HeroAnimState::Recover;
        heroAnim_  = Anim{};

        // SFX de recuperar ST
        if (gSound.ok()) {
            gSound.playSfx("recover_st");
        }

        // Aura amarela em volta do herói
        vfx_.type     = HeroVfxType::Buff;
        vfx_.duration = 0.45f;
        vfx_.t        = 0.f;
        vfx_.startX   = heroCenterX;
        vfx_.startY   = heroCenterY;
        vfx_.endX     = heroCenterX;
        vfx_.endY     = heroCenterY;
        vfx_.color    = SDL_Color{ 245, 220, 120, 50 }; // amarelo
    }

    // Se chegou aqui, a ação do player foi válida e a luta continua:
    // ficamos em PlayerTurn para mostrar o resultado e iniciamos o timer
    phase_ = Phase::PlayerTurn;
    playerResultTimer_ = 2.0f;  // <<< delay de 2 segundos antes do boss
}

void BattleScene::doEnemyAction() {
    // calcula o dano, mas não muda phase_ aqui
    lastDamageE_ = boss_->doTurn();
    playerHP_ = std::max(0, playerHP_ - lastDamageE_);

    // som do boss atacando (se de fato houver dano)
    if (lastDamageE_ > 0 && gSound.ok()) {
        gSound.playSfx("boss_attack");
    }

    if (playerHP_ <= 0)
        nextPhaseAfterEnemy_ = Phase::Lose;
    else
        nextPhaseAfterEnemy_ = Phase::Menu;
}

// --- marca boss derrotado no GameState com base em lastPortalId (ou nome) ---
void BattleScene::markBossDefeated() {
    if (!gs_) return;

    if (!gs_->lastPortalId.empty()) {
        const std::string& id = gs_->lastPortalId;
        if      (id == "furia")    gs_->deadFuria    = true;
        else if (id == "tempo")    gs_->deadTempo    = true;
        else if (id == "silencio") gs_->deadSilencio = true;
        else if (id == "orgulho")  gs_->deadOrgulho  = true;
    } else if (boss_) {
        std::string n = std::string(boss_->name());
        if      (n == "Furia")    gs_->deadFuria    = true;
        else if (n == "Tempo")    gs_->deadTempo    = true;
        else if (n == "Silencio") gs_->deadSilencio = true;
        else if (n == "Orgulho")  gs_->deadOrgulho  = true;
    }
}

void BattleScene::update(float dt) {
    if (!initialized_) { resetFromSet(); initialized_ = true; }

    // 0) Atualiza animação do HERÓI conforme estado atual
    int heroFrames = heroDef_.idleFrames;
    float heroFps  = 6.f;

    switch (heroState_) {
        case HeroAnimState::Idle:
            heroFrames = heroDef_.idleFrames;
            heroFps    = 6.f;
            break;
        case HeroAnimState::Attack1:
            heroFrames = heroDef_.atk1Frames;
            heroFps    = 10.f;
            break;
        case HeroAnimState::Attack2:
            heroFrames = heroDef_.atk2Frames;
            heroFps    = 10.f;
            break;
        case HeroAnimState::Potion:
        case HeroAnimState::Recover:
            heroFrames = heroDef_.idleFrames;
            heroFps    = 6.f;
            break;
    }

    heroAnim_.fps   = heroFps;
    heroAnim_.count = std::max(1, heroFrames);
    heroAnim_.update(dt);

    // Atualiza VFX do herói
    if (vfx_.type != HeroVfxType::None) {
        vfx_.t += dt;
        if (vfx_.t >= vfx_.duration) {
            vfx_.type = HeroVfxType::None;
        }
    }

    // 1) Espera do resultado do player antes do inimigo agir
    if (phase_ == Phase::PlayerTurn) {
        if (playerResultTimer_ > 0.f) {
            playerResultTimer_ -= dt;
            if (playerResultTimer_ <= 0.f) {
                // quando o timer acaba, inicia turno do inimigo (se o boss ainda estiver vivo)
                if (boss_->hp > 0) {
                    phase_              = Phase::EnemyTurn;
                    bossAnimState_      = BossAnimState::Idle;
                    bossAttackTimer_    = 0.f;
                    nextPhaseAfterEnemy_= Phase::Menu;
                } else {
                    phase_ = Phase::Win;
                }
            }
        }
    }

    // 2) Lógica do turno do inimigo + animação de ataque
    if (phase_ == Phase::EnemyTurn) {
        // Se ainda não começou a animação de ataque, inicia
        if (bossAnimState_ != BossAnimState::Attacking) {
            bossAnimState_   = BossAnimState::Attacking;
            bossAttackTimer_ = bossAttackDuration_;
            doEnemyAction();  // calcula dano uma única vez no começo do ataque
        } else {
            // já estamos atacando: contar o tempo da animação
            bossAttackTimer_ -= dt;
            if (bossAttackTimer_ <= 0.f) {
                bossAnimState_ = BossAnimState::Idle;
                phase_         = nextPhaseAfterEnemy_;

                // ao voltar para o menu, herói volta para Idle
                if (phase_ == Phase::Menu && heroState_ != HeroAnimState::Idle) {
                    heroState_ = HeroAnimState::Idle;
                    heroAnim_  = Anim{};
                }
            }
        }
    }

    // Atualiza animação do boss (idle x attack)
    SpriteInfo si = boss_->sprite();
    if (bossAnimState_ == BossAnimState::Attacking &&
        si.attackStartIdx >= 0 && si.attackCount > 0) {
        // animação de ataque
        animBoss_.fps   = 10.f;
        animBoss_.count = si.attackCount;
    } else {
        // idle (1 frame só)
        animBoss_.fps   = 4.f;
        animBoss_.count = 1;
    }
    animBoss_.update(dt);

    // outros timers visuais
    tBoss_ += dt;
    if (enrageFlash_ > 0.f) enrageFlash_ = std::max(0.f, enrageFlash_ - dt);

    // detectar transição para enrage
    bool enr = boss_->enraged();
    if (enr && !wasEnraged_) enrageFlash_ = 0.25f;
    wasEnraged_ = enr;
}

void BattleScene::drawBars(SDL_Renderer* r, int x, int y, int w, int h, float ratio) {
    SDL_Rect bg{ x,y,w,h };
    SDL_SetRenderDrawColor(r, 50, 50, 60, 255);
    SDL_RenderFillRect(r, &bg);
    SDL_Rect fg{ x+2, y+2, int((w-4) * std::max(0.f, std::min(1.f, ratio))), h-4 };
    SDL_SetRenderDrawColor(r, 200, 220, 240, 255);
    SDL_RenderFillRect(r, &fg);
    SDL_SetRenderDrawColor(r, 20, 20, 25, 255);
    SDL_RenderDrawRect(r, &bg);
}

void BattleScene::render(SDL_Renderer* r) {
    // --- Fundo / Background ---
    if (!bg_) {
        std::string path = "assets/backgrounds/default.png";
        auto n = std::string(boss_->name());
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

    // Barras player/boss
    drawBars(r, 80, 80, 240, 16, float(playerHP_) / playerHPMax_);
    drawBars(r, 950, 80, 240, 16, float(boss_->hp)  / boss_->maxHP());

    // -------- HERÓI (animação) --------
    // Carrega sheets do herói sob demanda
    if (!heroIdleSheet_.tex && !heroDef_.idlePath.empty()) {
        heroIdleSheet_ = loadSpriteSheet(r, heroDef_.idlePath, heroDef_.frameW, heroDef_.frameH);
    }
    if (!heroAtk1Sheet_.tex && !heroDef_.atk1Path.empty()) {
        heroAtk1Sheet_ = loadSpriteSheet(r, heroDef_.atk1Path, heroDef_.frameW, heroDef_.frameH);
    }
    if (!heroAtk2Sheet_.tex && !heroDef_.atk2Path.empty()) {
        heroAtk2Sheet_ = loadSpriteSheet(r, heroDef_.atk2Path, heroDef_.frameW, heroDef_.frameH);
    }

    SpriteSheet* heroSheet = &heroIdleSheet_;
    int frameCount = std::max(1, heroDef_.idleFrames);

    switch (heroState_) {
        case HeroAnimState::Idle:
            heroSheet   = &heroIdleSheet_;
            frameCount  = std::max(1, heroDef_.idleFrames);
            break;
        case HeroAnimState::Attack1:
            heroSheet   = &heroAtk1Sheet_;
            frameCount  = std::max(1, heroDef_.atk1Frames);
            break;
        case HeroAnimState::Attack2:
            heroSheet   = &heroAtk2Sheet_;
            frameCount  = std::max(1, heroDef_.atk2Frames);
            break;
        case HeroAnimState::Potion:
        case HeroAnimState::Recover:
            heroSheet   = &heroIdleSheet_;
            frameCount  = std::max(1, heroDef_.idleFrames);
            break;
    }

    int heroW = int(heroDef_.frameW * heroScale_);
    int heroH = int(heroDef_.frameH * heroScale_);
    SDL_Rect playerDst{ heroX_, heroY_, heroW, heroH };

    if (heroSheet && heroSheet->tex) {
        int frame = heroAnim_.frameOffset();
        if (frame >= frameCount) frame = frameCount - 1;
        SDL_Rect src = heroSheet->frameRect(frame);
        SDL_RenderCopy(r, heroSheet->tex, &src, &playerDst);
    } else {
        // fallback caso algo dê errado no carregamento
        SDL_SetRenderDrawColor(r, 200, 220, 240, 255);
        SDL_RenderFillRect(r, &playerDst);
    }

    // -------- BOSS --------
    if (!bossSheet_.tex) {
        SpriteInfo si0 = boss_->sprite();
        bossSheet_ = loadSpriteSheet(r, si0.path, si0.fw, si0.fh);
    }
    SpriteInfo si = boss_->sprite();
    int bossW = int(si.fw * bossScale_);
    int bossH = int(si.fh * bossScale_);
    SDL_Rect bossDst{ bossX_, bossY_, bossW, bossH };

    // Índice do frame do boss
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

    // -------- VFX do HERÓI (bolinhas) --------
    if (vfx_.type != HeroVfxType::None) {
        float alpha = (vfx_.duration > 0.f) ? std::clamp(vfx_.t / vfx_.duration, 0.f, 1.f) : 1.f;

        SDL_SetRenderDrawColor(r, vfx_.color.r, vfx_.color.g, vfx_.color.b, vfx_.color.a);

        if (vfx_.type == HeroVfxType::Slash) {
            // círculo roxo "explodindo" perto do boss
            int baseR = 50;
            int radius = int(baseR * (1.0f - 0.4f * alpha));
            drawFilledCircle(r, int(vfx_.startX), int(vfx_.startY), radius);
        }
        else if (vfx_.type == HeroVfxType::Projectile) {
            // bolinha roxa indo do herói até o boss
            float t = alpha;
            float px = vfx_.startX + (vfx_.endX - vfx_.startX) * t;
            float py = vfx_.startY + (vfx_.endY - vfx_.startY) * t;
            int radius = 10;
            drawFilledCircle(r, int(px), int(py), radius);
        }
        else if (vfx_.type == HeroVfxType::Buff) {
            // aura circular em volta do herói
            float pulse = 1.0f + 0.3f * std::sin(alpha * 3.14159f);
            int baseR = std::max(heroW, heroH) / 3;
            int radius = int(baseR * pulse);
            float heroCenterX = float(heroX_) + heroW * 0.5f;
            float heroCenterY = float(heroY_) + heroH * 0.5f;
            drawFilledCircle(r, int(heroCenterX), int(heroCenterY), radius);
        }
    }

    // -------- Textos principais --------
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

    // -------- Menu / submenus --------
    if (phase_ == Phase::Menu) {
        const char* labels[3] = {"Atacar","Poção HP","Recuperar ST"};
        for (int i=0;i<3;i++){
            SDL_Rect b{ 90, 390 + i*50, 200, 40 };
            if (i == menuIndex_) SDL_SetRenderDrawColor(r, 98, 0, 255, 255);
            else                 SDL_SetRenderDrawColor(r, 82, 86, 108, 255);
            SDL_RenderFillRect(r, &b);
            if (text_) {
                SDL_Color black{255,255,255,255};
                text_->draw(r, labels[i], b.x + 10, b.y + 10, black);
            }
        }
    } else if (phase_ == Phase::ChoosingAttack) {
        SDL_Rect b1{ 320, 390, 360, 40 };
        SDL_Rect b2{ 320, 440, 360, 40 };
        SDL_SetRenderDrawColor(r, atkIndex_==0?200:70, atkIndex_==0?200:70, atkIndex_==0?220:90, 255);
        SDL_RenderFillRect(r, &b1);
        SDL_SetRenderDrawColor(r, atkIndex_==1?200:70, atkIndex_==1?200:70, atkIndex_==1?220:90, 255);
        SDL_RenderFillRect(r, &b2);

        if (text_) {
            SDL_Color black{20,20,30,255};
            auto fmt = [](const Attack& a){
                return std::string(a.name) + "  (ST " + std::to_string(a.cost) + ", Dmg " + std::to_string(a.dmg) + ")";
            };
            text_->draw(r, fmt(atkA_), b1.x + 10, b1.y + 10, black);
            text_->draw(r, fmt(atkB_), b2.x + 10, b2.y + 10, black);
        }
    } else if (phase_ == Phase::Win || phase_ == Phase::Lose) {
        if (text_) {
            SDL_Color white{245,245,255,255};
            text_->draw(
                r,
                phase_==Phase::Win ? "Vitoria! (Enter/Esc)" : "Derrota... (Enter/Esc)",
                280, 420,
                white
            );
        }
    }

    // ------------------------------
    // Mensagens separadas por tipo
    // ------------------------------
    if (text_) {
        // Mensagens de erro (vermelho)
        if (!errorMsg_.empty()) {
            SDL_Color red{220, 60, 60, 255};
            int errX = 320;
            int errY = 360;
            text_->draw(r, errorMsg_, errX, errY, red);
        }

        // Mensagens de info (branco)
        if (!infoMsg_.empty()) {
            SDL_Color white{245, 245, 255, 255};
            int infoX = 320;
            int infoY = 400;
            text_->draw(r, infoMsg_, infoX, infoY, white);
        }

        // Feedback de dano (amarelo) – próximo ao boss
        if (!dmgMsg_.empty()) {
            SDL_Color yellow{240, 220, 120, 255};
            int dmgX = 540;  // perto do boss
            int dmgY = 140;
            text_->draw(r, dmgMsg_, dmgX, dmgY, yellow);
        }
    }

    SDL_RenderPresent(r);
}
