#pragma once
#include "Boss.h"
#include <algorithm>

class BossTempo : public Boss {
    int turn_ = 0;
public:
    BossTempo() { hp = maxHP(); }
    const char* name() const override { return "Tempo"; }
    int maxHP() const override { return 90; }
    int doTurn() override {
        // começa alto e decai: 14,12,10,9,8,... piso 4
        int dmg = std::max(4, 14 - 2*turn_);
        ++turn_;
        return dmg;
    }
    SpriteInfo sprite() const override {
        return SpriteInfo{ "assets/sprites/bosses/tempo.png", 96,96, 3,4, 0, -1 };
    }
};
