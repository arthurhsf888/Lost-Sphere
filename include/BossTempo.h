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
        int dmg = 1;

        if (turn_ == 0)      dmg = 10;
        else if (turn_ == 1) dmg = 15;
        else if (turn_ == 2) dmg = 25;
        else                 dmg = 1;

        turn_++;
        return dmg;
    }

    SpriteInfo sprite() const override {
        return SpriteInfo{
            "assets/sprites/bosses/tempo.png",
            64,64,
            4,4,
            4,
            -1,
            4,
            4
        };
    }
};
