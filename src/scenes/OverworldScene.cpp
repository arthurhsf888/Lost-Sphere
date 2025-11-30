#include "OverworldScene.h"
#include <SDL.h>
#include <string>   // para std::string nos rótulos

OverworldScene::~OverworldScene() {
    if (playerSheet_.tex) SDL_DestroyTexture(playerSheet_.tex);
    if (portalSheet_.tex) SDL_DestroyTexture(portalSheet_.tex);
}

bool OverworldScene::aabbIntersect(const SDL_FRect& a, const SDL_Rect& b) {
    return !(a.x + a.w <= b.x || a.x >= b.x + b.w ||
             a.y + a.h <= b.y || a.y >= b.y + b.h);
}

// Verifica se o retângulo r toca algum tile sólido do mapa
bool OverworldScene::collidesWithSolidTiles(const SDL_FRect& r) const {
    if (!mapLoaded_) return false;

    const int tileW = map_.tileW();
    const int tileH = map_.tileH();

    const int inset = 1; // checa levemente para dentro do retângulo

    int px[4] = {
        int(r.x)               + inset,
        int(r.x + r.w) - 1     - inset,
        int(r.x)               + inset,
        int(r.x + r.w) - 1     - inset
    };
    int py[4] = {
        int(r.y)               + inset,
        int(r.y)               + inset,
        int(r.y + r.h) - 1     - inset,
        int(r.y + r.h) - 1     - inset
    };

    for (int i = 0; i < 4; ++i) {
        if (map_.isSolidAtPixel(px[i], py[i])) {
            return true;
        }
    }
    return false;
}

void OverworldScene::resolveCollisions(SDL_FRect& next) {
    // Colisão separada por eixo: primeiro X, depois Y
    SDL_FRect test = player_;

    // tenta mover em X
    test.x = next.x;
    if (!collidesWithSolidTiles(test)) {
        player_.x = test.x;
    }

    // tenta mover em Y (usando o X já atualizado)
    test.y = next.y;
    if (!collidesWithSolidTiles(test)) {
        player_.y = test.y;
    }
}

void OverworldScene::handleEvent(const SDL_Event& e) {
    if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) {
        const bool down = (e.type == SDL_KEYDOWN);
        switch (e.key.keysym.sym) {
            case SDLK_a: case SDLK_LEFT:
                vx_ = down ? -1.f : (vx_ < 0 ? 0.f : vx_);
                if (down) facing_ = Dir::Left;
                break;
            case SDLK_d: case SDLK_RIGHT:
                vx_ = down ?  1.f : (vx_ > 0 ? 0.f : vx_);
                if (down) facing_ = Dir::Right;
                break;
            case SDLK_w: case SDLK_UP:
                vy_ = down ? -1.f : (vy_ < 0 ? 0.f : vy_);
                if (down) facing_ = Dir::Up;
                break;
            case SDLK_s: case SDLK_DOWN:
                vy_ = down ?  1.f : (vy_ > 0 ? 0.f : vy_);
                if (down) facing_ = Dir::Down;
                break;

            case SDLK_ESCAPE:
                sm_.setActive("overworld");
                break;

            case SDLK_e: {
                // Encostando em algum portal → define batalha e vai para seleção de classe
                for (const auto& p : portals_) {
                    if (aabbIntersect(player_, p.rect)) {
                        if (gs_) {
                            gs_->nextBattleSceneId = p.battleSceneId; // batalha alvo
                        }
                        sm_.setActive("selectset");
                        break;
                    }
                }
            } break;
        }
    }
}

int OverworldScene::baseIndexForDir(Dir d) const {
    const int cols = playerSheet_.cols;

    int row = 2; // default Down
    switch (d) {
        case Dir::Up:    row = 0; break;
        case Dir::Left:  row = 1; break;
        case Dir::Down:  row = 2; break;
        case Dir::Right: row = 3; break;
    }
    return row * cols;
}

void OverworldScene::update(float dt) {
    SDL_FRect next = player_;
    next.x += vx_ * speed_ * dt;
    next.y += vy_ * speed_ * dt;
    resolveCollisions(next);

    moving_ = (vx_ != 0.f || vy_ != 0.f);

    anim_.fps = 8.f;   // velocidade da caminhada
    anim_.update(dt);
}

void OverworldScene::render(SDL_Renderer* r) {
    SDL_SetRenderDrawColor(r, 12, 18, 26, 255);
    SDL_RenderClear(r);

    // --- Mapa via TileMap: overworld.png + overworld.csv ---
    if (!mapLoaded_) {
        mapLoaded_ = map_.load(
            r,
            "assets/tiles/overworld.png",
            "assets/tiles/overworld.csv",
            32, 32,
            8  // 256x256 => 8 colunas de 32px
        );
    }

    if (mapLoaded_) {
        map_.render(r);
    }

    // carregar sheet do player (uma vez)
    if (!playerSheet_.tex) {
        const int FRAME_W = 32;
        const int FRAME_H = 32;
        playerSheet_ = loadSpriteSheet(
            r,
            "assets/sprites/player/player.png", // spritesheet 736x128
            FRAME_W, FRAME_H
        );
        if (playerSheet_.tex) {
            player_.w = (float)FRAME_W;
            player_.h = (float)FRAME_H;
        }
    }

    // carregar sheet dos portais (uma vez)
    if (!portalSheet_.tex) {
        portalSheet_ = loadSpriteSheet(
            r,
            "assets/sprites/portal/portal.png",
            32, 32
        );
    }

    // desenhar portais + rótulos
    if (portalSheet_.tex) {
        const int frameIdx = 0; // usa sempre o primeiro portal

        for (const auto& p : portals_) {
            SDL_Rect src = portalSheet_.frameRect(frameIdx);
            SDL_Rect dst = p.rect;
            SDL_RenderCopy(r, portalSheet_.tex, &src, &dst);

            if (text_) {
                int tx = dst.x + dst.w/2 - 6;
                int ty = dst.y + dst.h/2 - 10;
                std::string s(1, p.label);
                text_->draw(r, s, tx, ty);
            }
        }
    }

    // ---- desenhar player com animação Idle/Run/Costas + flip + scale 120% ----
    if (playerSheet_.tex && playerSheet_.cols > 0 && playerSheet_.rows > 0) {
        const int cols = playerSheet_.cols;

        // Você informou (1-based):
        //  - Parado:   col = 1,  row = 3
        //  - Correndo: col = 2,  row = 3
        //  - De costas:col = 19, row = 3
        const int row1      = 3;
        const int colIdle1  = 1;
        const int colRun1   = 4;
        const int colBack1  = 19;

        // converte para 0-based
        const int row     = row1 - 1;
        const int colIdle = colIdle1 - 1;
        const int colRun  = colRun1  - 1;
        const int colBack = colBack1 - 1;

        int col = colIdle;
        SDL_RendererFlip flip = SDL_FLIP_NONE;

        if (facing_ == Dir::Up) {
            // sprite de costas (sem flip)
            col  = colBack;
            flip = SDL_FLIP_NONE;
        } else {
            // frente/lado: usamos sempre a animação "de lado" (direita)
            // e espelhamos quando estiver indo pra esquerda.
            if (moving_) col = colRun;
            else         col = colIdle;

            if (facing_ == Dir::Left) {
                flip = SDL_FLIP_HORIZONTAL;
            } else {
                // Right / Down usam o sprite normal (de lado pra direita)
                flip = SDL_FLIP_NONE;
            }
        }

        int idx = row * cols + col;
        SDL_Rect src = playerSheet_.frameRect(idx);

        // escala visual 120% (sem mexer na hitbox / colisão)
        const float SCALE = 1.5f;
        SDL_Rect dst;
        dst.w = int(player_.w * SCALE);
        dst.h = int(player_.h * SCALE);
        dst.x = int(player_.x - (dst.w - player_.w) / 2);
        dst.y = int(player_.y - (dst.h - player_.h) / 2);

        SDL_RenderCopyEx(r, playerSheet_.tex, &src, &dst, 0.0, nullptr, flip);
    } else {
        SDL_Rect pr{ (int)player_.x, (int)player_.y, (int)player_.w, (int)player_.h };
        SDL_SetRenderDrawColor(r, 230, 230, 240, 255);
        SDL_RenderFillRect(r, &pr);
    }

    SDL_RenderPresent(r);
}
