#pragma once
#include <string>

enum class PlayerSet { Guerreiro, Mago, Cacador };

struct GameState {
    PlayerSet set = PlayerSet::Guerreiro;
    int potions   = 2;

    // stats base (você pode balancear depois)
    int hpMax   = 50;
    int stMax   = 20;
    int atkBase = 5;

    // cena de batalha para onde devemos ir após escolher o set
    std::string nextBattleSceneId = "battle_furia";

    // --- NOVO: flags de bosses derrotados ---
    bool deadFuria    = false;
    bool deadTempo    = false;
    bool deadSilencio = false;
    bool deadOrgulho  = false;

    // --- NOVO: portal que levou à batalha atual ---
    std::string lastPortalId;

    // --- NOVO: todos os quatro foram derrotados? ---
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
