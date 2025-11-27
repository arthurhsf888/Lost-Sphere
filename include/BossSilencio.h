#pragma once
#include "Boss.h"
#include <algorithm>

class BossSilencio : public Boss {
    int turn_ = 0;
public:
    BossSilencio() { hp = maxHP(); }
    const char* name() const override { return "Silencio"; }
    int maxHP() const override { return 85; }
    int doTurn() override {
        // sobe: 3,5,7,9,... teto 15
        int dmg = std::min(15, 3 + 2*turn_);
        ++turn_;
        return dmg;
    }
    SpriteInfo sprite() const override {
        return SpriteInfo{ "assets/sprites/bosses/silencio.png", 96,96, 3,4, 0, -1 };
    }
};
