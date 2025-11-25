#pragma once
#include <string>

enum class PlayerSet { Guerreiro, Mago, Cacador };

struct GameState {
    PlayerSet set = PlayerSet::Guerreiro;
    int potions = 2;

    // stats base (você pode balancear depois)
    int hpMax = 50;
    int stMax = 20;
    int atkBase = 5;

    std::string setName() const {
        switch(set){
            case PlayerSet::Guerreiro: return "Guerreiro";
            case PlayerSet::Mago:      return "Mago";
            default:                   return "Caçador";
        }
    }
};
