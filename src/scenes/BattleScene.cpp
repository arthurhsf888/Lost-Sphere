#include "BattleScene.h"
#include <SDL.h>
#include <algorithm>
#include <cmath>

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

  phase_ = Phase::Menu;
  menuIndex_ = 0;
  atkIndex_ = 0;
  lastDamageP_ = lastDamageE_ = 0;
  lastMsg_.clear();
}

void BattleScene::handleEvent(const SDL_Event& e) {
  if (!initialized_) { resetFromSet(); initialized_ = true; }

  if (e.type != SDL_KEYDOWN) return;

  if (phase_ == Phase::Menu) {
    if (e.key.keysym.sym == SDLK_UP)   menuIndex_ = (menuIndex_ + 2) % 3;
    if (e.key.keysym.sym == SDLK_DOWN) menuIndex_ = (menuIndex_ + 1) % 3;
    if (e.key.keysym.sym == SDLK_RETURN) {
      if (menuIndex_ == 0) { phase_ = Phase::ChoosingAttack; }
      else if (menuIndex_ == 1) { phase_ = Phase::PlayerTurn; doPlayerAction(); }
      else if (menuIndex_ == 2) { phase_ = Phase::PlayerTurn; doPlayerAction(); }
    }
    if (e.key.keysym.sym == SDLK_ESCAPE) { initialized_ = false; sm_.setActive("overworld"); }
  }
  else if (phase_ == Phase::ChoosingAttack) {
    if (e.key.keysym.sym == SDLK_UP || e.key.keysym.sym == SDLK_DOWN)
      atkIndex_ = 1 - atkIndex_;
    if (e.key.keysym.sym == SDLK_ESCAPE) phase_ = Phase::Menu;
    if (e.key.keysym.sym == SDLK_RETURN) { phase_ = Phase::PlayerTurn; doPlayerAction(); }
  }
  else if (phase_ == Phase::Win || phase_ == Phase::Lose) {
    if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_ESCAPE) {
      initialized_ = false;
      sm_.setActive("overworld");
    }
  }
}

void BattleScene::doPlayerAction() {
  lastDamageP_ = 0;
  lastMsg_.clear();

  if (menuIndex_ == 0) {
    const Attack& atk = (atkIndex_ == 0 ? atkA_ : atkB_);
    if (playerST_ >= atk.cost) {
      playerST_ -= atk.cost;
      // aplica multiplicador de fraqueza/resistência do boss
      float mult = Balance::bossFuriaMultiplier(atk.type);
      lastDamageP_ = std::max(1, int(std::round(atk.dmg * mult)));
      boss_->hp = std::max(0, boss_->hp - lastDamageP_);

      if (mult > 1.01f)      lastMsg_ = "Foi super eficaz!";
      else if (mult < 0.99f) lastMsg_ = "Pouco eficaz...";
      else                   lastMsg_.clear();
    } else {
      lastMsg_ = "Stamina insuficiente!";
    }
  } else if (menuIndex_ == 1) {
    if (gs_->potions > 0) {
      gs_->potions--;
      playerHP_ = std::min(playerHPMax_, playerHP_ + 12);
      lastMsg_ = "Voce usou uma Pocao.";
    } else {
      lastMsg_ = "Sem pocoes!";
    }
  } else if (menuIndex_ == 2) {
    playerST_ = std::min(playerSTMax_, playerST_ + 4);
    lastMsg_ = "Voce respirou e recuperou ST.";
  }

  if (boss_->hp <= 0) phase_ = Phase::Win;
  else phase_ = Phase::EnemyTurn;
}

void BattleScene::doEnemyAction() {
  lastDamageE_ = boss_->doTurn();
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

  // Barras player/boss
  drawBars(r, 80, 80, 240, 16, float(playerHP_) / playerHPMax_);
  drawBars(r, 480, 80, 240, 16, float(boss_->hp)  / boss_->maxHP());

  if (text_) {
    text_->draw(r, "Chefe: " + std::string(boss_->name()) + (boss_->enraged() ? " (ENRAGE)" : ""), 480, 104);
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
    SDL_Rect b1{ 320, 390, 360, 40 };
    SDL_Rect b2{ 320, 440, 360, 40 };
    SDL_SetRenderDrawColor(r, atkIndex_==0?200:70, atkIndex_==0?200:70, atkIndex_==0?220:90, 255); SDL_RenderFillRect(r, &b1);
    SDL_SetRenderDrawColor(r, atkIndex_==1?200:70, atkIndex_==1?200:70, atkIndex_==1?220:90, 255); SDL_RenderFillRect(r, &b2);

    if (text_) {
      auto fmt = [](const Attack& a){
        return std::string(a.name) + "  (ST " + std::to_string(a.cost) + ", Dmg " + std::to_string(a.dmg) + ")";
      };
      text_->draw(r, fmt(atkA_), b1.x + 10, b1.y + 10);
      text_->draw(r, fmt(atkB_), b2.x + 10, b2.y + 10);
    }
  } else if (phase_ == Phase::Win || phase_ == Phase::Lose) {
    if (text_) text_->draw(r, phase_==Phase::Win ? "Vitoria! (Enter/Esc)" : "Derrota... (Enter/Esc)", 280, 420);
  }

  // mensagens/feedback
  if (text_ && !lastMsg_.empty()) text_->draw(r, lastMsg_, 320, 360);

  // impactos visuais simples
  if (lastDamageP_ > 0) { SDL_Rect hit{ 540, 140, 40, 40 }; SDL_SetRenderDrawColor(r, 220,120,120,255); SDL_RenderFillRect(r, &hit); }
  if (lastDamageE_ > 0) { SDL_Rect hit{ 160, 180, 40, 40 }; SDL_SetRenderDrawColor(r, 120,120,220,255); SDL_RenderFillRect(r, &hit); }

  SDL_RenderPresent(r);
}
