#pragma once
#include <string>

// Informações de spritesheet de cada boss
struct SpriteInfo {
    std::string path;
    int fw   = 96;
    int fh   = 96;
    int cols = 3;
    int rows = 4;

    int idleIdx   = 0;
    int enrageIdx = -1;

    // Configuração da sequência de ataque
    int attackStartIdx = -1;
    int attackCount    = 0;
};

// Classe base para todos os bosses
class Boss {
public:
    int hp = 0;
    virtual ~Boss() = default;

    virtual const char* name() const = 0;
    virtual int  maxHP() const = 0;
    virtual bool enraged() const { return false; }
    virtual int  doTurn() = 0;
    virtual SpriteInfo sprite() const = 0;
};
