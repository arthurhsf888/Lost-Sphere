#pragma once

enum class DamageType { Fisico, Magico, Distancia };

struct Balance {
    // multiplicadores do Boss Fúria vs tipo do golpe
    static float bossFuriaMultiplier(DamageType t) {
        switch (t) {
            case DamageType::Fisico:     return 1.0f; // neutro
            case DamageType::Magico:     return 1.2f; // fraco contra magia
            case DamageType::Distancia:  return 0.8f; // resistente a distância
        }
        return 1.0f;
    }
};
