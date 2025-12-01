#pragma once
#include <string>

enum class PlayerSet { Guerreiro, Mago, Cacador };

struct GameState {
    PlayerSet set = PlayerSet::Guerreiro;
    int potions   = 2;

    int hpMax   = 50;
    int stMax   = 20;
    int atkBase = 5;

    std::string nextBattleSceneId = "battle_furia";

    bool deadFuria    = false;
    bool deadTempo    = false;
    bool deadSilencio = false;
    bool deadOrgulho  = false;

    int bossesDefeated() const {
        int c = 0;
        if (deadFuria)    c++;
        if (deadTempo)    c++;
        if (deadSilencio) c++;
        if (deadOrgulho)  c++;
        return c;
    }

    std::string lastPortalId;

    bool allBossesDefeated() const {
        return deadFuria && deadTempo && deadSilencio && deadOrgulho;
    }

    std::string setName() const {
        switch (set) {
            case PlayerSet::Guerreiro: return "Guerreiro";
            case PlayerSet::Mago:      return "Mago";
            default:                   return "Caçador";
        }
    }
};
