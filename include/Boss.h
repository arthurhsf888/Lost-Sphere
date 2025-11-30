#pragma once
#include <string>

struct SpriteInfo {
    std::string path;

    // Tamanho de cada frame
    int fw   = 96;
    int fh   = 96;

    // Layout do spritesheet
    int cols = 3;
    int rows = 4;

    // Frame “parado”
    int idleIdx   = 0;

    // Frame especial de enrage; -1 se não usar
    int enrageIdx = -1;

    // --- NOVO: animação de ataque ---
    // Índice inicial (0-based) da sequência de ataque
    // e quantos frames ela possui.
    // Se attackStartIdx < 0 ou attackCount <= 0, não anima ataque.
    int attackStartIdx = -1;
    int attackCount    = 0;
};

class Boss {
public:
    int hp = 0;
    virtual ~Boss() = default;

    virtual const char* name() const = 0;
    virtual int  maxHP() const = 0;
    virtual bool enraged() const { return false; }
    virtual int  doTurn() = 0;              // dano causado ao player
    virtual SpriteInfo sprite() const = 0;  // infos visuais p/ BattleScene
};
