#include "OverworldScene.h"
#include <SDL.h>
#include <string>
#include "../SoundManager.h"

// instância global declarada em outro lugar (ex: main.cpp)
extern SoundManager gSound;

OverworldScene::~OverworldScene() {
    if (playerSheet_.tex) SDL_DestroyTexture(playerSheet_.tex);
    if (portalSheet_.tex) SDL_DestroyTexture(portalSheet_.tex);
    if (doorTex_)         SDL_DestroyTexture(doorTex_);
}

bool OverworldScene::aabbIntersect(const SDL_FRect& a, const SDL_Rect& b) {
    return !(a.x + a.w <= b.x || a.x >= b.x + b.w ||
             a.y + a.h <= b.y || a.y >= b.y + b.h);
}

// Verifica se o retângulo r toca algum tile sólido do mapa
bool OverworldScene::collidesWithSolidTiles(const SDL_FRect& r) const {
    if (!mapLoaded_) return false;

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
                // aqui você pode trocar para "menu" se tiver
                sm_.setActive("overworld");
                break;

            case SDLK_e: {
                // 1) Porta final
                if (aabbIntersect(player_, door_)) {
                    if (gs_ && gs_->allBossesDefeated()) {
                        // toca música de jogo zerado
                        if (gSound.ok()) {
                            gSound.playMusic("game_clear", 0);
                        }
                        msgBox_.show("Voce voltou para casa! Fim da jornada.", 4.0f);
                        // aqui você poderia ir para uma cena de créditos/menu_final
                        // ex: sm_.setActive("menu_final");
                    } else {
                        msgBox_.show("A porta esta trancada...");
                    }
                    break;
                }

                // 2) Portais
                for (const auto& p : portals_) {
                    if (!aabbIntersect(player_, p.rect))
                        continue;

                    if (!gs_) {
                        sm_.setActive("selectset");
                        break;
                    }

                    // checar se este portal já foi limpado
                    bool defeated = false;
                    if      (p.id == "furia")    defeated = gs_->deadFuria;
                    else if (p.id == "tempo")    defeated = gs_->deadTempo;
                    else if (p.id == "silencio") defeated = gs_->deadSilencio;
                    else if (p.id == "orgulho")  defeated = gs_->deadOrgulho;

                    if (defeated) {
                        msgBox_.show("Portal dissipado.");
                        break;
                    }

                    // Portal ainda ativo → som de entrar no portal + seleção de classe
                    if (gSound.ok()) {
                        gSound.playSfx("enter_portal");
                    }
                    gs_->nextBattleSceneId = p.battleSceneId;
                    gs_->lastPortalId      = p.id;
                    sm_.setActive("selectset");
                    break;
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
    // Garantir que a música do overworld esteja tocando
    if (!musicStarted_) {
        if (gSound.ok()) {
            gSound.playMusic("overworld_theme", -1);
        }
        musicStarted_ = true;
    }

    SDL_FRect next = player_;
    next.x += vx_ * speed_ * dt;
    next.y += vy_ * speed_ * dt;
    resolveCollisions(next);

    bool wasMoving = moving_;
    moving_ = (vx_ != 0.f || vy_ != 0.f);

    anim_.fps = 8.f;   // velocidade da caminhada
    anim_.update(dt);

    // Passos do herói
    if (moving_) {
        stepTimer_ -= dt;
        if (stepTimer_ <= 0.f) {
            if (gSound.ok()) {
                // alterna entre passo1 e passo2
                gSound.playSfx(stepToggle_ ? "step2" : "step1");
            }
            stepToggle_ = !stepToggle_;
            stepTimer_ = 0.35f; // intervalo entre passos (ajuste à vontade)
        }
    } else {
        // parou de andar -> reinicia timer pra primeiro passo sair rápido
        if (wasMoving) {
            stepTimer_ = 0.f;
        }
    }

    // atualiza mensagem temporária
    msgBox_.update(dt);
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
        const int FRAME_W = 48;
        const int FRAME_H = 48;
        playerSheet_ = loadSpriteSheet(
            r,
            "assets/sprites/player/george.png",
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

    // carregar textura da porta final (uma vez)
    if (!doorTex_) {
        doorTex_ = loadTexture(r, "assets/sprites/portal/door.png");
    }

    // --- Porta final como sprite ---
    if (doorTex_) {
        SDL_RenderCopy(r, doorTex_, nullptr, &door_);
    } else {
        // fallback: retângulo amarelo se textura não carregar
        SDL_SetRenderDrawColor(r, 200, 180, 40, 255);
        SDL_RenderFillRect(r, &door_);
    }

    // desenhar portais + rótulos (somente se ainda não derrotados)
    if (portalSheet_.tex) {
        const int frameIdx = 0; // usa sempre o primeiro portal

        for (const auto& p : portals_) {
            bool defeated = false;
            if (gs_) {
                if      (p.id == "furia")    defeated = gs_->deadFuria;
                else if (p.id == "tempo")    defeated = gs_->deadTempo;
                else if (p.id == "silencio") defeated = gs_->deadSilencio;
                else if (p.id == "orgulho")  defeated = gs_->deadOrgulho;
            }

            if (defeated) {
                // portal dissipado: não desenhar
                continue;
            }

            SDL_Rect src = portalSheet_.frameRect(frameIdx);
            SDL_Rect dst = p.rect;
            SDL_RenderCopy(r, portalSheet_.tex, &src, &dst);

            if (text_) {
                SDL_Color white{230,230,240,255};
                int tx = dst.x + dst.w/2 - 6;
                int ty = dst.y + dst.h/2 - 10;
                std::string s(1, p.label);
                text_->draw(r, s, tx, ty, white);
            }
        }
    }

    // ---- desenhar player com animação Idle/Run/Costas + flip + scale 150% ----
    if (playerSheet_.tex && playerSheet_.cols > 0 && playerSheet_.rows > 0) {
        const int cols = playerSheet_.cols;

        // Você informou (1-based):
        //  - Parado:   col = 1,  row = 3
        //  - Correndo: col = 4,  row = 3
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
            col  = colBack;
            flip = SDL_FLIP_NONE;
        } else {
            if (moving_) col = colRun;
            else         col = colIdle;

            if (facing_ == Dir::Left) {
                flip = SDL_FLIP_HORIZONTAL;
            } else {
                flip = SDL_FLIP_NONE;
            }
        }

        int idx = row * cols + col;
        SDL_Rect src = playerSheet_.frameRect(idx);

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

    // --- Mensagem temporária (Portal dissipado, porta trancada, fim, etc) ---
    msgBox_.render(r, text_, 480, 40);

    SDL_RenderPresent(r);
}
