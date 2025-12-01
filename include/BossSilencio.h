#pragma once
#include "Boss.h"
#include <algorithm>

class BossSilencio : public Boss {
    int turn_ = 0;

public:
    BossSilencio() { hp = maxHP(); }

    void reset() { turn_ = 0; }

    const char* name() const override { return "Silencio"; }
    int maxHP() const override { return 30; }

    int doTurn() override {
        int dmg = 10 * (1 << turn_);
        turn_++;
        return dmg;
    }

    SpriteInfo sprite() const override {
        return SpriteInfo{
            "assets/sprites/bosses/silencio.png",
            64,64,
            4,4,
            4,
            -1,
            4,
            4
        };
    }
};
