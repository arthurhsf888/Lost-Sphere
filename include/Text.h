#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <string>

class Text {
public:
    Text() = default;

    ~Text() {
        if (font_) {
            TTF_CloseFont(font_);
            font_ = nullptr;
        }
    }

    bool init(const std::string& fontPath, int pt) {
        if (!TTF_WasInit() && TTF_Init() != 0) {
            return false;
        }
        font_ = TTF_OpenFont(fontPath.c_str(), pt);
        return font_ != nullptr;
    }

    void draw(SDL_Renderer* r, const std::string& s, int x, int y) {
        SDL_Color col{230, 230, 240, 255};
        draw(r, s, x, y, col);
    }

    void draw(SDL_Renderer* r,
              const std::string& s,
              int x, int y,
              SDL_Color col) {
        if (!font_) return;

        SDL_Surface* surf = TTF_RenderUTF8_Blended(font_, s.c_str(), col);
        if (!surf) return;

        SDL_Texture* tex = SDL_CreateTextureFromSurface(r, surf);
        SDL_Rect dst{ x, y, surf->w, surf->h };

        SDL_FreeSurface(surf);
        if (!tex) return;

        SDL_RenderCopy(r, tex, nullptr, &dst);
        SDL_DestroyTexture(tex);
    }

private:
    TTF_Font* font_ = nullptr;
};
