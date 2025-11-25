#pragma once
#include "Boss.h"

class BossFuria : public Boss {
public:
    const char* name() const override { return "Furia"; }
    const char* intro() const override { return "A ira toma forma... Prepare-se!"; }

    int maxHP() const override        { return 48; }
    int attackBase() const override   { return 6;  }
    int enrageThreshold() const override { return 30; } // <=30%
    int enrageBonus() const override  { return 3;  }
};
