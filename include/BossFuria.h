#pragma once
#include "Boss.h"
#include <algorithm>

class BossFuria : public Boss {
public:
    BossFuria() { hp = maxHP(); }

    const char* name() const override { return "Furia"; }
    int maxHP() const override { return 70; }

    bool enraged() const override {
        return hp <= int(0.30 * maxHP());
    }

    int doTurn() override {
        return enraged() ? 10 : 5;
    }

    SpriteInfo sprite() const override {
        return SpriteInfo{
            "assets/sprites/bosses/furia.png",
            64,64,
            4,4,
            4,
            1,
            4,
            4
        };
    }
};
