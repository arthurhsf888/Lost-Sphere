#pragma once
#include "Boss.h"
#include <algorithm>

class BossOrgulho : public Boss {
public:
    BossOrgulho() { hp = maxHP(); }
    const char* name() const override { return "Orgulho"; }
    int maxHP() const override { return 100; }
    int doTurn() override {
        // sangra 5 de HP por rodada
        hp = std::max(0, hp - 5);
        // dano direto pequeno
        return 4;
    }
    SpriteInfo sprite() const override {
        return SpriteInfo{ "assets/sprites/bosses/orgulho.png", 96,96, 3,4, 0, -1 };
    }
};
