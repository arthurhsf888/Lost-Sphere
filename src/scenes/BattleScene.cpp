#include "BattleScene.h"
#include <SDL.h>
#include <algorithm>

void BattleScene::resetFromSet() {
  // stats base
  playerHPMax_ = gs_->hpMax;
  playerSTMax_ = gs_->stMax;

  // variações por set
  switch (gs_->set) {
    case PlayerSet::Guerreiro:
      playerHPMax_ += 10;             // mais robusto
      atkA_ = {"Corte Rapido", 3, 6};
      atkB_ = {"Golpe Pesado", 6, 12};
      break;
    case PlayerSet::Mago:
      atkA_ = {"Bola de Fogo", 5, 10};
      atkB_ = {"Raio",         4,  8};
      break;
    case PlayerSet::Cacador:
      playerSTMax_ += 4;              // melhor gestão de stamina
      atkA_ = {"Flecha Precisa", 4, 7};
      atkB_ = {"Tiro Triplo",    6, 11};
      break;
  }

  playerHP_ = playerHPMax_;
  playerST_ = playerSTMax_;
  enemyHP_ = enemyHPMax_ = 40; // por enquanto fixo
  phase_ = Phase::Menu;
  menuIndex_ = 0;
  atkIndex_ = 0;
  lastDamageP_ = lastDamageE_ = 0;
}

void BattleScene::handleEvent(const SDL_Event& e) {
  if (!initialized_) { resetFromSet(); initialized_ = true; }

  if (e.type != SDL_KEYDOWN) return;

  if (phase_ == Phase::Menu) {
    if (e.key.keysym.sym == SDLK_UP)   menuIndex_ = (menuIndex_ + 2) % 3;
    if (e.key.keysym.sym == SDLK_DOWN) menuIndex_ = (menuIndex_ + 1) % 3;
    if (e.key.keysym.sym == SDLK_RETURN) {
      if (menuIndex_ == 0) { phase_ = Phase::ChoosingAttack; } // Atacar -> escolher golpe
      else if (menuIndex_ == 1) { phase_ = Phase::PlayerTurn; doPlayerAction(); } // Poção
      else if (menuIndex_ == 2) { phase_ = Phase::PlayerTurn; doPlayerAction(); } // Desviar
    }
    if (e.key.keysym.sym == SDLK_ESCAPE) sm_.setActive("overworld");
  }
  else if (phase_ == Phase::ChoosingAttack) {
    if (e.key.keysym.sym == SDLK_UP || e.key.keysym.sym == SDLK_DOWN)
      atkIndex_ = 1 - atkIndex_;
    if (e.key.keysym.sym == SDLK_ESCAPE) phase_ = Phase::Menu;
    if (e.key.keysym.sym == SDLK_RETURN) {
      phase_ = Phase::PlayerTurn;
      doPlayerAction();
    }
  }
  else if (phase_ == Phase::Win || phase_ == Phase::Lose) {
    if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_ESCAPE) {
      initialized_ = false; // reseta na próxima vez
      sm_.setActive("overworld");
    }
  }
}

void BattleScene::doPlayerAction() {
  lastDamageP_ = 0;

  if (menuIndex_ == 0) {
    // Atacar -> usar atkIndex_
    const Attack& atk = (atkIndex_ == 0 ? atkA_ : atkB_);
    if (playerST_ >= atk.cost) {
      playerST_ -= atk.cost;
      lastDamageP_ = atk.dmg;
      enemyHP_ = std::max(0, enemyHP_ - lastDamageP_);
    }
  } else if (menuIndex_ == 1) {
    // Poção (usa do GameState)
    if (gs_->potions > 0) {
      gs_->potions--;
      playerHP_ = std::min(playerHPMax_, playerHP_ + 12);
    }
  } else if (menuIndex_ == 2) {
    // Desviar: recupera stamina
    playerST_ = std::min(playerSTMax_, playerST_ + 4);
  }

  if (enemyHP_ <= 0) phase_ = Phase::Win;
  else phase_ = Phase::EnemyTurn;
}

void BattleScene::doEnemyAction() {
  // inimigo placeholder: dano fixo, poderia variar por “enrage”
  lastDamageE_ = 5;
  playerHP_ = std::max(0, playerHP_ - lastDamageE_);
  if (playerHP_ <= 0) phase_ = Phase::Lose;
  else phase_ = Phase::Menu;
}

void BattleScene::update(float) {
  if (phase_ == Phase::EnemyTurn) doEnemyAction();
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
  if (!initialized_) { resetFromSet(); initialized_ = true; }

  SDL_SetRenderDrawColor(r, 18, 12, 20, 255);
  SDL_RenderClear(r);

  // Arena
  SDL_Rect arena{ 60, 60, 680, 280 };
  SDL_SetRenderDrawColor(r, 35, 25, 45, 255);
  SDL_RenderFillRect(r, &arena);

  // Barras
  drawBars(r, 80, 80, 240, 16, float(playerHP_) / playerHPMax_);
  drawBars(r, 480, 80, 240, 16, float(enemyHP_) / enemyHPMax_);

  // Textos
  if (text_) {
    text_->draw(r, "Set: " + gs_->setName(), 80, 104);
    text_->draw(r, "HP: " + std::to_string(playerHP_) + "/" + std::to_string(playerHPMax_), 80, 124);
    text_->draw(r, "ST: " + std::to_string(playerST_) + "/" + std::to_string(playerSTMax_), 80, 144);
    text_->draw(r, "Pocoes: " + std::to_string(gs_->potions), 80, 164);
  }

  // Painel inferior
  SDL_Rect panel{ 60, 360, 680, 180 };
  SDL_SetRenderDrawColor(r, 25, 25, 35, 255);
  SDL_RenderFillRect(r, &panel);

  if (phase_ == Phase::Menu) {
    const char* labels[3] = {"Atacar","Pocao","Desviar"};
    for (int i=0;i<3;i++){
      SDL_Rect b{ 90, 390 + i*50, 200, 40 };
      if (i == menuIndex_) SDL_SetRenderDrawColor(r, 200, 200, 220, 255);
      else                 SDL_SetRenderDrawColor(r, 70, 70, 90, 255);
      SDL_RenderFillRect(r, &b);
      if (text_) text_->draw(r, labels[i], b.x + 10, b.y + 10);
    }
  } else if (phase_ == Phase::ChoosingAttack) {
    // submenu de golpes
    SDL_Rect b1{ 320, 390, 360, 40 };
    SDL_Rect b2{ 320, 440, 360, 40 };

    // CORRIGIR estas duas linhas:
    SDL_SetRenderDrawColor(r,
      atkIndex_==0 ? 200 : 70,
      atkIndex_==0 ? 200 : 70,
      atkIndex_==0 ? 220 : 90, 255);
    SDL_RenderFillRect(r, &b1);

    SDL_SetRenderDrawColor(r,
      atkIndex_==1 ? 200 : 70,
      atkIndex_==1 ? 200 : 70,
      atkIndex_==1 ? 220 : 90, 255);
    SDL_RenderFillRect(r, &b2);

    if (text_) {
      text_->draw(r, std::string(atkA_.name) + "  (ST " + std::to_string(atkA_.cost) + ", Dmg " + std::to_string(atkA_.dmg) + ")", b1.x + 10, b1.y + 10);
      text_->draw(r, std::string(atkB_.name) + "  (ST " + std::to_string(atkB_.cost) + ", Dmg " + std::to_string(atkB_.dmg) + ")", b2.x + 10, b2.y + 10);
    }
  } else if (phase_ == Phase::Win || phase_ == Phase::Lose) {
    if (text_) text_->draw(r, phase_==Phase::Win ? "Vitoria! (Enter/Esc)" : "Derrota... (Enter/Esc)", 280, 420);
  }

  // impactos visuais simples
  if (lastDamageP_ > 0) { SDL_Rect hit{ 540, 140, 40, 40 }; SDL_SetRenderDrawColor(r, 220,120,120,255); SDL_RenderFillRect(r, &hit); }
  if (lastDamageE_ > 0) { SDL_Rect hit{ 160, 180, 40, 40 }; SDL_SetRenderDrawColor(r, 120,120,220,255); SDL_RenderFillRect(r, &hit); }

  SDL_RenderPresent(r);
}
