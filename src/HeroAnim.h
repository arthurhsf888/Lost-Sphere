#pragma once
#include <string>
#include "GameState.h"

struct HeroAnimDef {
    std::string idlePath;
    int idleFrames = 1;

    std::string atk1Path;
    int atk1Frames = 1;

    std::string atk2Path;
    int atk2Frames = 1;

    int frameW = 128;
    int frameH = 128;
};

inline HeroAnimDef heroAnimFor(PlayerSet set) {
    HeroAnimDef def;
    def.frameW = 128;
    def.frameH = 128;

    switch (set) {
        case PlayerSet::Cacador:
            def.idlePath    = "assets/sprites/player/cacador_idle.png";
            def.idleFrames  = 4;
            def.atk1Path    = "assets/sprites/player/cacador_habilidade1.png";
            def.atk1Frames  = 14;
            def.atk2Path    = "assets/sprites/player/cacador_habilidade2.png";
            def.atk2Frames  = 13;
            break;

        case PlayerSet::Mago:
            def.idlePath    = "assets/sprites/player/mago_idle.png";
            def.idleFrames  = 5;
            def.atk1Path    = "assets/sprites/player/mago_habilidade1.png";
            def.atk1Frames  = 10;
            def.atk2Path    = "assets/sprites/player/mago_habilidade2.png";
            def.atk2Frames  = 7;
            break;

        case PlayerSet::Guerreiro:
        default:
            def.idlePath    = "assets/sprites/player/guerreiro_idle.png";
            def.idleFrames  = 8;
            def.atk1Path    = "assets/sprites/player/guerreiro_habilidade1.png";
            def.atk1Frames  = 6;
            def.atk2Path    = "assets/sprites/player/guerreiro_habilidade2.png";
            def.atk2Frames  = 4;
            break;
    }

    return def;
}
