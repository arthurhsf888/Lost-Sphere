#pragma once
#include <string>
#include "GameState.h"

// Definição de sprites do herói por classe
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

// Retorna a configuração de sprites para cada set
inline HeroAnimDef heroAnimFor(PlayerSet set) {
    HeroAnimDef def;
    def.frameW = 128;
    def.frameH = 128;

    switch (set) {
        case PlayerSet::Cacador:
            // caçador:
            // - idle: 512x128 => 4 frames
            // - hab1: 1792x128 => 14 frames
            // - hab2: 1664x128 => 13 frames
            def.idlePath    = "assets/sprites/player/cacador_idle.png";
            def.idleFrames  = 4;
            def.atk1Path    = "assets/sprites/player/cacador_habilidade1.png";
            def.atk1Frames  = 14;
            def.atk2Path    = "assets/sprites/player/cacador_habilidade2.png";
            def.atk2Frames  = 13;
            break;

        case PlayerSet::Mago:
            // mago:
            // - idle: 640x128 => 5 frames
            // - hab1: 1280x128 => 10 frames
            // - hab2: 896x128 => 7 frames
            def.idlePath    = "assets/sprites/player/mago_idle.png";
            def.idleFrames  = 5;
            def.atk1Path    = "assets/sprites/player/mago_habilidade1.png";
            def.atk1Frames  = 10;
            def.atk2Path    = "assets/sprites/player/mago_habilidade2.png";
            def.atk2Frames  = 7;
            break;

        case PlayerSet::Guerreiro:
        default:
            // guerreiro:
            // - idle: 1024x128 => 8 frames
            // - hab1: 768x128 => 6 frames
            // - hab2: 512x128 => 4 frames
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
