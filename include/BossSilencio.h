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
        // spritesheet 256x256, 4x4; idle = idx 0; sem enrage; ataque = linha 2 (idx 4..7)
        return SpriteInfo{
            "assets/sprites/bosses/silencio.png",
            64, 64,
            4, 4,
            4,   // idleIdx
            -1,  // enrageIdx
            4,   // attackStartIdx
            4    // attackCount
        };
    }
};
