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
        // spritesheet 256x256, 4x4; idle = idx 0; sem enrage; ataque = linha 2 (idx 4..7)
        return SpriteInfo{
            "assets/sprites/bosses/orgulho.png", // path
            64, 64,                             // fw, fh
            4, 4,                                 // cols, rows
            4,                                    // idleIdx
            -1,                                   // enrageIdx (não usa)
            4,                                    // attackStartIdx (linha 2, col 0)
            4                                     // attackCount (4 frames: 4,5,6,7)
        };
    }
};
