#pragma once

enum class DamageType { Fisico, Magico, Distancia };

// Tabela de multiplicadores de dano contra bosses
struct Balance {
    static float bossFuriaMultiplier(DamageType t) {
        switch (t) {
            case DamageType::Fisico:     return 1.0f;
            case DamageType::Magico:     return 1.0f;
            case DamageType::Distancia:  return 1.0f;
        }
        return 1.0f;
    }
};
