#include "../include/SpriteSheet.h"
#include "../include/Assets.h"

#if __has_include(<SDL2/SDL_image.h>)
  #include <SDL2/SDL_image.h>
#else
  #include <SDL_image.h>
#endif

SpriteSheet loadSpriteSheet(SDL_Renderer* r, const std::string& path, int frameW, int frameH) {
    SpriteSheet sh;
    sh.fw = frameW;
    sh.fh = frameH;
    sh.tex = loadTexture(r, path);

    if (sh.tex) {
        SDL_QueryTexture(sh.tex, nullptr, nullptr, &sh.texW, &sh.texH);
        sh.cols = sh.texW / sh.fw;
        sh.rows = sh.texH / sh.fh;
    }
    return sh;
}

void drawFrame(SDL_Renderer* r, const SpriteSheet& sh, int idx, int x, int y, float scale) {
    if (!sh.tex || sh.cols == 0 || sh.rows == 0) return;
    SDL_Rect src = sh.frameRect(idx);
    SDL_Rect dst { x, y, int(src.w * scale), int(src.h * scale) };
    SDL_RenderCopy(r, sh.tex, &src, &dst);
}
