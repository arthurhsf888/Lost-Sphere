#pragma once
#include <string>

struct SpriteInfo {
    std::string path;
    int fw = 96, fh = 96;  // frame width/height
    int cols = 3, rows = 4;
    int idleIdx = 0;       // frame “parado”
    int enrageIdx = -1;    // frame enrage; -1 se não usar
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
