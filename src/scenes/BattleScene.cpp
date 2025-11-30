#include "BattleScene.h"
#include <SDL.h>
#include <algorithm>
#include <cmath>

BattleScene::~BattleScene() {
  if (texPlayer_)     SDL_DestroyTexture(texPlayer_);
  if (bossSheet_.tex) SDL_DestroyTexture(bossSheet_.tex);
  if (bg_)            SDL_DestroyTexture(bg_);
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

  wasEnraged_      = boss_->enraged();
  enrageFlash_     = 0.f;
  bossAnimState_   = BossAnimState::Idle;
  bossAttackTimer_ = 0.f;
  playerResultTimer_ = 0.f;
}

void BattleScene::handleEvent(const SDL_Event& e) {
  if (!initialized_) { resetFromSet(); initialized_ = true; }

  if (e.type != SDL_KEYDOWN) return;

  // ---------------- FASE: MENU PRINCIPAL ----------------
  if (phase_ == Phase::Menu) {
    if (e.key.keysym.sym == SDLK_UP)   menuIndex_ = (menuIndex_ + 2) % 3;
    if (e.key.keysym.sym == SDLK_DOWN) menuIndex_ = (menuIndex_ + 1) % 3;

    if (e.key.keysym.sym == SDLK_RETURN) {
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

    if (e.key.keysym.sym == SDLK_ESCAPE) {
      initialized_ = false;
      sm_.setActive("overworld");
    }
  }
  // ---------------- FASE: ESCOLHER ATAQUE ----------------
  else if (phase_ == Phase::ChoosingAttack) {
    if (e.key.keysym.sym == SDLK_UP || e.key.keysym.sym == SDLK_DOWN)
      atkIndex_ = 1 - atkIndex_;

    if (e.key.keysym.sym == SDLK_ESCAPE)
      phase_ = Phase::Menu;

    if (e.key.keysym.sym == SDLK_RETURN) {
      phase_ = Phase::PlayerTurn;
      doPlayerAction();   // depois dessa função ficamos em PlayerTurn, Win ou Menu (erro ST)
    }
  }
  // ---------------- FASE: RESULTADO FINAL ----------------
  else if (phase_ == Phase::Win || phase_ == Phase::Lose) {
    if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_ESCAPE) {
      initialized_ = false;
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

  // --- Ação "Atacar" ---
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

    // Orgulho reflete total do dano recebido
    /*
    if (std::string(boss_->name()) == "Orgulho" && lastDamageP_ > 0) {
      playerHP_ = std::max(0, playerHP_ - lastDamageP_);
      if (!dmgMsg_.empty()) dmgMsg_ += " ";
      dmgMsg_ += "O chefe refletiu o dano!";
    }

    // feedback de eficácia
    if (mult > 1.01f) {
      if (!dmgMsg_.empty()) dmgMsg_ += " ";
      dmgMsg_ += "Foi super eficaz!";
    } else if (mult < 0.99f) {
      if (!dmgMsg_.empty()) dmgMsg_ += " ";
      dmgMsg_ += "Pouco eficaz...";
    }*/

    // Chefe morreu -> termina batalha
    if (boss_->hp <= 0) {
      phase_ = Phase::Win;
      return;
    }
  }
  // --- Ação "Pocao" ---
  else if (menuIndex_ == 1) {
    if (gs_->potions > 0) {
      gs_->potions--;
      playerHP_ = std::min(playerHPMax_, playerHP_ + 12);
      infoMsg_ = "Voce usou uma Pocao.";
    } else {
      errorMsg_ = "Sem pocoes!";
      phase_ = Phase::Menu;
      return;
    }
  }
  // --- Ação "Recuperar" ---
  else if (menuIndex_ == 2) {
    playerST_ = std::min(playerSTMax_, playerST_ + 4);
    infoMsg_ = "Voce respirou e recuperou ST.";
  }

  // Se chegou aqui, a ação do player foi válida e a luta continua:
  // ficamos em PlayerTurn para mostrar o resultado e iniciamos o timer
  phase_ = Phase::PlayerTurn;
  playerResultTimer_ = 2.0f;  // <<< aqui está o delay de 2 segundos
}

void BattleScene::doEnemyAction() {
  // calcula o dano, mas não muda phase_ aqui
  lastDamageE_ = boss_->doTurn();
  playerHP_ = std::max(0, playerHP_ - lastDamageE_);

  if (playerHP_ <= 0)
    nextPhaseAfterEnemy_ = Phase::Lose;
  else
    nextPhaseAfterEnemy_ = Phase::Menu;
}

void BattleScene::update(float dt) {
  // 1) Espera do resultado do player antes do inimigo agir
  if (phase_ == Phase::PlayerTurn) {
    if (playerResultTimer_ > 0.f) {
      playerResultTimer_ -= dt;
      if (playerResultTimer_ <= 0.f) {
        // quando o timer acaba, inicia turno do inimigo (se o boss ainda estiver vivo)
        if (boss_->hp > 0) {
          phase_             = Phase::EnemyTurn;
          bossAnimState_     = BossAnimState::Idle;
          bossAttackTimer_   = 0.f;
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

  // Player
  if (!texPlayer_) texPlayer_ = loadTexture(r, "assets/sprites/player/battle_idle.png");
  SDL_Rect playerDst{ 340, 350, 64, 64 };
  if (texPlayer_) SDL_RenderCopy(r, texPlayer_, nullptr, &playerDst);
  else {
    SDL_SetRenderDrawColor(r, 200, 220, 240, 255);
    SDL_RenderFillRect(r, &playerDst);
  }

  // Boss via SpriteSheet
  if (!bossSheet_.tex) {
    SpriteInfo si0 = boss_->sprite();
    bossSheet_ = loadSpriteSheet(r, si0.path, si0.fw, si0.fh);
  }
  SpriteInfo si = boss_->sprite();
  SDL_Rect bossDst{ 740, 260, si.fw * 3, si.fh * 3 };

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

  // Textos principais
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

  // Menu / submenus
  if (phase_ == Phase::Menu) {
    const char* labels[3] = {"Atacar","Pocao","Recuperar"};
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

  // impactos visuais simples
  /*if (lastDamageP_ > 0) {
    SDL_Rect hit{ 540, 140, 40, 40 };
    SDL_SetRenderDrawColor(r, 220,120,120,255);
    SDL_RenderFillRect(r, &hit);
  }
  if (lastDamageE_ > 0) {
    SDL_Rect hit{ 160, 180, 40, 40 };
    SDL_SetRenderDrawColor(r, 120,120,220,255);
    SDL_RenderFillRect(r, &hit);
  }*/

  SDL_RenderPresent(r);
}