#pragma once
#include <SDL.h>
#include <string>

struct SpriteSheet {
    SDL_Texture* tex = nullptr;
    int texW = 0, texH = 0;
    int fw = 96, fh = 96;
    int cols = 0, rows = 0;

    SDL_Rect frameRect(int idx) const {
        const int n = cols * rows;
        if (n == 0) return SDL_Rect{0,0,0,0};
        idx %= n; if (idx < 0) idx += n;
        int c = idx % cols;
        int r = idx / cols;
        return SDL_Rect{ c * fw, r * fh, fw, fh };
    }
};

SpriteSheet loadSpriteSheet(SDL_Renderer* r,
                            const std::string& path,
                            int frameW, int frameH);

void drawFrame(SDL_Renderer* r,
               const SpriteSheet& sh,
               int idx, int x, int y,
               float scale=1.0f);
