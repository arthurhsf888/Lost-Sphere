#pragma once
#include "Boss.h"
#include <algorithm>

class BossFuria : public Boss {
public:
    BossFuria() { hp = maxHP(); }
    const char* name() const override { return "Furia"; }
    int maxHP() const override { return 80; }
    bool enraged() const override { return hp <= int(0.30 * maxHP()); }
    int doTurn() override {
        // causa mais dano quando enrage
        return enraged() ? 10 : 6;
    }
    SpriteInfo sprite() const override {
        // seu sheet: 288x384 (96x96), 3x4; idle = (row=3,col=0), enrage = (row=3,col=1)
        return SpriteInfo{ "assets/sprites/bosses/furia.png", 96,96, 3,4, 9, 10 };
    }
};
