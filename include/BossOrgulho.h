#pragma once
#include "Boss.h"
#include <algorithm>

class BossOrgulho : public Boss {
public:
    BossOrgulho() { hp = maxHP(); }

    const char* name() const override { return "Orgulho"; }
    int maxHP() const override { return 100; }

    int doTurn() override {
        hp = std::min(maxHP(), hp + 1);
        return 3;
    }

    SpriteInfo sprite() const override {
        return SpriteInfo{
            "assets/sprites/bosses/orgulho.png",
            64,64,
            4,4,
            4,
            -1,
            4,
            4
        };
    }
};
