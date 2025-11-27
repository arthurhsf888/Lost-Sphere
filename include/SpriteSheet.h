#pragma once
#include <SDL.h>
#include <string>

struct SpriteSheet {
    SDL_Texture* tex = nullptr;
    int texW = 0, texH = 0;   // tamanho do PNG
    int fw = 96, fh = 96;     // tamanho de cada frame
    int cols = 0, rows = 0;   // calculados

    // Retorna o retângulo de origem do frame idx (0..cols*rows-1)
    SDL_Rect frameRect(int idx) const {
        const int n = cols * rows;
        if (n == 0) return SDL_Rect{0,0,0,0};
        idx %= n; if (idx < 0) idx += n;
        int c = idx % cols;
        int r = idx / cols;
        return SDL_Rect{ c * fw, r * fh, fw, fh };
    }
};

// Carrega a textura e preenche cols/rows a partir das dimensões da imagem.
SpriteSheet loadSpriteSheet(SDL_Renderer* r, const std::string& path, int frameW, int frameH);

// Desenhar um frame em (x,y) com escala (1.0 = 96×96 na tela)
void drawFrame(SDL_Renderer* r, const SpriteSheet& sh, int idx, int x, int y, float scale=1.0f);
