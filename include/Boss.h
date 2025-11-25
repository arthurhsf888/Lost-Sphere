#pragma once
#include "Balance.h"
#include <string>

class Boss {
public:
    virtual ~Boss() = default;

    virtual const char* name() const = 0;
    virtual const char* intro() const = 0;

    virtual int  maxHP() const = 0;
    virtual int  attackBase() const = 0;         // dano base por turno
    virtual int  enrageThreshold() const = 0;    // % (0..100)
    virtual int  enrageBonus() const = 0;        // +dano quando enraged

    // Estado
    int hp = 0;

    // Entra em "enrage" quando HP <= threshold%
    bool enraged() const {
        return hp <= (maxHP() * enrageThreshold() / 100);
    }

    // Dano do boss este turno (padrão = 1 ataque fixo)
    virtual int doTurn() {
        return (enraged() ? attackBase() + enrageBonus() : attackBase());
    }
};
