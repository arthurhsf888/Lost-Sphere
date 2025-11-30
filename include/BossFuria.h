#pragma once
#include "Boss.h"
#include <algorithm>

class BossFuria : public Boss {
public:
    BossFuria() { hp = maxHP(); }
    const char* name() const override { return "Furia"; }
    int maxHP() const override { return 80; }

    bool enraged() const override {
        return hp <= int(0.30 * maxHP());
    }

    int doTurn() override {
        // causa mais dano quando enrage
        return enraged() ? 10 : 6;
    }

    SpriteInfo sprite() const override {
        // spritesheet 256x256, 4x4
        // idle   = idx 0  (linha 1, col 0)
        // enrage = idx 1  (linha 1, col 1)
        // ataque = linha 2 (idx 4..7)
        return SpriteInfo{
            "assets/sprites/bosses/furia.png",
            64, 64,
            4, 4,
            4,   // idleIdx
            1,   // enrageIdx
            4,   // attackStartIdx (linha 2)
            4    // attackCount
        };
    }
};
