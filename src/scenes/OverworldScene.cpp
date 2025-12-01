#include "OverworldScene.h"
#include <SDL.h>
#include <string>
#include "../SoundManager.h"

// -----------------------------------------------------------------------------
// Som global (definido em outro lugar, como main.cpp)
// -----------------------------------------------------------------------------
extern SoundManager gSound;

// -----------------------------------------------------------------------------
// Destrutor: libera texturas carregadas dinamicamente
// -----------------------------------------------------------------------------
OverworldScene::~OverworldScene() {
    if (playerSheet_.tex) SDL_DestroyTexture(playerSheet_.tex);
    if (portalSheet_.tex) SDL_DestroyTexture(portalSheet_.tex);
    if (doorTex_)         SDL_DestroyTexture(doorTex_);
    if (storyIntroTex_)   SDL_DestroyTexture(storyIntroTex_);
    if (storyEndingTex_)  SDL_DestroyTexture(storyEndingTex_);
}

// -----------------------------------------------------------------------------
// AABB simples entre SDL_FRect (player) e SDL_Rect (portal/porta)
// -----------------------------------------------------------------------------
bool OverworldScene::aabbIntersect(const SDL_FRect& a, const SDL_Rect& b) {
    return !(a.x + a.w <= b.x || a.x >= b.x + b.w ||
             a.y + a.h <= b.y || a.y >= b.y + b.h);
}

// -----------------------------------------------------------------------------
// Verifica se o player colide com tiles sólidos do mapa (usando TileMap)
// -----------------------------------------------------------------------------
bool OverworldScene::collidesWithSolidTiles(const SDL_FRect& r) const {
    if (!mapLoaded_) return false;

    const int inset = 1; // encosta um pouquinho pra dentro para evitar stuck

    // Testa os 4 cantos
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

// -----------------------------------------------------------------------------
// Ajusta movimento do player separando colisões em X e Y
// -----------------------------------------------------------------------------
void OverworldScene::resolveCollisions(SDL_FRect& next) {
    SDL_FRect test = player_;

    // tenta mover em X
    test.x = next.x;
    if (!collidesWithSolidTiles(test)) {
        player_.x = test.x;
    }

    // tenta mover em Y
    test.y = next.y;
    if (!collidesWithSolidTiles(test)) {
        player_.y = test.y;
    }
}

// -----------------------------------------------------------------------------
// Evento de teclado do Overworld
// - movimentação WASD
// - entrar em portal com 'E'
// - overlays de história interceptam eventos
// -----------------------------------------------------------------------------
void OverworldScene::handleEvent(const SDL_Event& e) {

    // Se overlay de história está ativo, só aceita Enter/Space/E
    if (e.type == SDL_KEYDOWN &&
        (overlay_ == OverlayState::Intro || overlay_ == OverlayState::Ending)) {

        SDL_Keycode key = e.key.keysym.sym;
        if (key == SDLK_RETURN || key == SDLK_SPACE || key == SDLK_e) {

            // Sai do overlay de introdução
            if (overlay_ == OverlayState::Intro) {
                overlay_ = OverlayState::None;
            }
            // Sai do overlay final e volta ao menu
            else if (overlay_ == OverlayState::Ending) {
                overlay_ = OverlayState::None;
                sm_.setActive("menu");
            }
        }
        return; // overlay ativo → ignora todo resto
    }

    // -------------------------------------------------------------------------
    // Movimentação + Interação
    // -------------------------------------------------------------------------
    if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) {

        bool down = (e.type == SDL_KEYDOWN);

        switch (e.key.keysym.sym) {

            // Movimento horizontal
            case SDLK_a: case SDLK_LEFT:
                vx_ = down ? -1.f : (vx_ < 0 ? 0.f : vx_);
                if (down) facing_ = Dir::Left;
                break;

            case SDLK_d: case SDLK_RIGHT:
                vx_ = down ?  1.f : (vx_ > 0 ? 0.f : vx_);
                if (down) facing_ = Dir::Right;
                break;

            // Movimento vertical
            case SDLK_w: case SDLK_UP:
                vy_ = down ? -1.f : (vy_ < 0 ? 0.f : vy_);
                if (down) facing_ = Dir::Up;
                break;

            case SDLK_s: case SDLK_DOWN:
                vy_ = down ?  1.f : (vy_ > 0 ? 0.f : vy_);
                if (down) facing_ = Dir::Down;
                break;

            // ESC → poderia voltar para menu (mas aqui só recarrega overworld)
            case SDLK_ESCAPE:
                sm_.setActive("overworld");
                break;

            // -----------------------------------------------------------------
            // Interação com 'E'
            // -----------------------------------------------------------------
            case SDLK_e: {

                // 1) Interação com a Porta Final
                if (aabbIntersect(player_, door_)) {

                    // todos os bosses mortos → mostra ending
                    if (gs_ && gs_->allBossesDefeated()) {
                        if (gSound.ok()) gSound.playMusic("game_clear", 0);
                        vx_ = vy_ = 0.f;
                        overlay_ = OverlayState::Ending;
                    } else {
                        msgBox_.show("A porta está trancada...");
                    }
                    break;
                }

                // 2) Interação com Portais de Boss
                for (const auto& p : portals_) {

                    if (!aabbIntersect(player_, p.rect))
                        continue;

                    if (!gs_) {
                        sm_.setActive("selectset");
                        break;
                    }

                    // Checar se portal já derrotado
                    bool defeated = false;
                    if      (p.id == "furia")    defeated = gs_->deadFuria;
                    else if (p.id == "tempo")    defeated = gs_->deadTempo;
                    else if (p.id == "silencio") defeated = gs_->deadSilencio;
                    else if (p.id == "orgulho")  defeated = gs_->deadOrgulho;

                    if (defeated) {
                        msgBox_.show("Portal dissipado.");
                        break;
                    }

                    // Portal ativo → entrar na escolha de classe
                    if (gSound.ok()) gSound.playSfx("enter_portal");

                    gs_->nextBattleSceneId = p.battleSceneId;
                    gs_->lastPortalId      = p.id;
                    sm_.setActive("selectset");
                    break;
                }
            } break;
        }
    }
}

// -----------------------------------------------------------------------------
// Converte direção em índice base na spritesheet do player
// -----------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------
// Atualização do Overworld:
// - Música
// - Movimento + colisão
// - Animação do player
// - SFX de passos
// - Detecta proximidade de portais → nearPortal_
// - Atualiza mensagens temporárias
// -----------------------------------------------------------------------------
void OverworldScene::update(float dt) {

    // Garantir música do overworld
    if (!musicStarted_) {
        if (gSound.ok()) gSound.playMusic("overworld_theme", -1);
        musicStarted_ = true;
    }

    // Overlays bloqueiam movimento
    if (overlay_ == OverlayState::Intro || overlay_ == OverlayState::Ending) {
        msgBox_.update(dt);
        nearPortal_ = false;
        return;
    }

    // Movimento tentado
    SDL_FRect next = player_;
    next.x += vx_ * speed_ * dt;
    next.y += vy_ * speed_ * dt;
    resolveCollisions(next);

    bool wasMoving = moving_;
    moving_ = (vx_ != 0.f || vy_ != 0.f);

    anim_.fps = 8.f;
    anim_.update(dt);

    // Som de passos
    if (moving_) {
        stepTimer_ -= dt;
        if (stepTimer_ <= 0.f) {
            if (gSound.ok()) {
                gSound.playSfx(stepToggle_ ? "step2" : "step1");
            }
            stepToggle_ = !stepToggle_;
            stepTimer_ = 0.35f;
        }
    }
    else if (wasMoving) {
        stepTimer_ = 0.f;
    }

    // -------------------------------------------------------------------------
    // Detectar proximidade de portais (para exibir prompt “Aperte E...”)
    // -------------------------------------------------------------------------
    nearPortal_ = false;

    for (const auto& p : portals_) {

        bool defeated = false;
        if (gs_) {
            if      (p.id == "furia")    defeated = gs_->deadFuria;
            else if (p.id == "tempo")    defeated = gs_->deadTempo;
            else if (p.id == "silencio") defeated = gs_->deadSilencio;
            else if (p.id == "orgulho")  defeated = gs_->deadOrgulho;
        }
        if (defeated) continue;

        // área expandida para detectar proximidade (não precisa encostar)
        SDL_Rect expanded = p.rect;
        const int margin = 12;
        expanded.x -= margin;
        expanded.y -= margin;
        expanded.w += margin * 2;
        expanded.h += margin * 2;

        if (aabbIntersect(player_, expanded)) {
            nearPortal_ = true;
            break;
        }
    }

    // Atualiza mensagem temporária
    msgBox_.update(dt);
}

// -----------------------------------------------------------------------------
// Renderização completa do Overworld:
// - Mapa
// - Player
// - Portais
// - Porta final
// - Overlays de história
// - HUD (skull)
// - Prompt "Aperte E para entrar no portal"
// -----------------------------------------------------------------------------
void OverworldScene::render(SDL_Renderer* r) {

    SDL_SetRenderDrawColor(r, 12, 18, 26, 255);
    SDL_RenderClear(r);

    // -------------------------------------------------------------------------
    // Mapa
    // -------------------------------------------------------------------------
    if (!mapLoaded_) {
        mapLoaded_ = map_.load(
            r, "assets/tiles/overworld.png",
            "assets/tiles/overworld.csv",
            32, 32,
            8
        );
    }

    if (mapLoaded_) map_.render(r);

    // -------------------------------------------------------------------------
    // Player sheet
    // -------------------------------------------------------------------------
    if (!playerSheet_.tex) {
        const int FRAME_W = 48;
        const int FRAME_H = 48;
        playerSheet_ = loadSpriteSheet(
            r,
            "assets/sprites/player/george.png",
            FRAME_W, FRAME_H
        );
        if (playerSheet_.tex) {
            player_.w = float(FRAME_W);
            player_.h = float(FRAME_H);
        }
    }

    // -------------------------------------------------------------------------
    // Portal sheet
    // -------------------------------------------------------------------------
    if (!portalSheet_.tex) {
        portalSheet_ = loadSpriteSheet(
            r,
            "assets/sprites/portal/portal.png",
            32, 32
        );
    }

    // -------------------------------------------------------------------------
    // Porta final
    // -------------------------------------------------------------------------
    if (!doorTex_) {
        doorTex_ = loadTexture(r, "assets/sprites/portal/door.png");
    }

    if (doorTex_) SDL_RenderCopy(r, doorTex_, nullptr, &door_);
    else {
        SDL_SetRenderDrawColor(r, 200, 180, 40, 255);
        SDL_RenderFillRect(r, &door_);
    }

    // -------------------------------------------------------------------------
    // Portais ativos (não derrotados)
    // -------------------------------------------------------------------------
    if (portalSheet_.tex) {

        const int frameIdx = 0;

        for (const auto& p : portals_) {

            bool defeated = false;
            if (gs_) {
                if      (p.id == "furia")    defeated = gs_->deadFuria;
                else if (p.id == "tempo")    defeated = gs_->deadTempo;
                else if (p.id == "silencio") defeated = gs_->deadSilencio;
                else if (p.id == "orgulho")  defeated = gs_->deadOrgulho;
            }
            if (defeated) continue;

            SDL_Rect src = portalSheet_.frameRect(frameIdx);
            SDL_Rect dst = p.rect;

            SDL_RenderCopy(r, portalSheet_.tex, &src, &dst);

            // rótulo (F, T, S, O)
            if (text_) {
                SDL_Color white{230,230,240,255};
                int tx = dst.x + dst.w/2 - 6;
                int ty = dst.y + dst.h/2 - 10;
                text_->draw(r, std::string(1, p.label), tx, ty, white);
            }
        }
    }

    // -------------------------------------------------------------------------
    // Player (com flip horizontal)
    // -------------------------------------------------------------------------
    if (playerSheet_.tex && playerSheet_.cols > 0 && playerSheet_.rows > 0) {

        const int cols = playerSheet_.cols;

        const int row1      = 3;
        const int colIdle1  = 1;
        const int colRun1   = 4;
        const int colBack1  = 19;

        const int row     = row1 - 1;
        const int colIdle = colIdle1 - 1;
        const int colRun  = colRun1  - 1;
        const int colBack = colBack1 - 1;

        int col = colIdle;
        SDL_RendererFlip flip = SDL_FLIP_NONE;

        if (facing_ == Dir::Up) {
            col  = colBack;
            flip = SDL_FLIP_NONE;
        }
        else {
            col = (moving_ ? colRun : colIdle);
            flip = (facing_ == Dir::Left ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
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
    }
    else {
        SDL_Rect pr{ int(player_.x), int(player_.y), int(player_.w), int(player_.h) };
        SDL_SetRenderDrawColor(r, 230, 230, 240, 255);
        SDL_RenderFillRect(r, &pr);
    }

    // -------------------------------------------------------------------------
    // Mensagem temporária (MessageBox)
    // -------------------------------------------------------------------------
    msgBox_.render(r, text_, 1000, 350);

    // -------------------------------------------------------------------------
    // Overlays de história (intro / ending)
    // -------------------------------------------------------------------------
    if (overlay_ == OverlayState::Intro) {

        if (!storyIntroTex_) {
            storyIntroTex_ = loadTexture(r, "assets/ui/story_intro.png");
        }

        if (storyIntroTex_) {
            int w = 600, h = 300;
            SDL_Rect dst{ (VW - w)/2, (VH - h)/2, w, h };
            SDL_RenderCopy(r, storyIntroTex_, nullptr, &dst);
        }
    }
    else if (overlay_ == OverlayState::Ending) {

        if (!storyEndingTex_) {
            storyEndingTex_ = loadTexture(r, "assets/ui/story_ending.png");
        }

        if (storyEndingTex_) {
            int w = 900, h = 300;
            SDL_Rect dst{ (VW - w)/2, (VH - h)/2, w, h };
            SDL_RenderCopy(r, storyEndingTex_, nullptr, &dst);
        }
    }

    // -------------------------------------------------------------------------
    // HUD — caveira + contagem de bosses derrotados
    // -------------------------------------------------------------------------
    if (!skullTex_) {
        skullTex_ = loadTexture(r, "assets/ui/skull.png");
    }

    if (skullTex_ && text_) {

        int defeated = gs_ ? gs_->bossesDefeated() : 0;

        SDL_Rect icon{ 400, 20, 42, 42 };
        SDL_RenderCopy(r, skullTex_, nullptr, &icon);

        SDL_Color white{235, 240, 245, 255};
        std::string msg = "Portais concluídos: " +
                          std::to_string(defeated) + "/4";

        text_->draw(r, msg, icon.x + icon.w + 12, icon.y + 10, white);
    }

    // -------------------------------------------------------------------------
    // Prompt “Aperte E para entrar no portal”
    // -------------------------------------------------------------------------
    if (overlay_ == OverlayState::None && nearPortal_ && text_) {

        int boxW = 300;
        int boxH = 40;

        // posição à direita do player
        int px = int(player_.x + player_.w) + 12;
        int py = int(player_.y) - 10;

        // ajusta para não sair da tela
        if (px + boxW > VW - 10) px = VW - boxW - 10;
        if (py < 10)             py = 10;
        if (py + boxH > VH - 10) py = VH - boxH - 10;

        SDL_Rect box{ px, py, boxW, boxH };

        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(r, 40, 90, 200, 230);
        SDL_RenderFillRect(r, &box);

        SDL_SetRenderDrawColor(r, 10, 40, 120, 255);
        SDL_RenderDrawRect(r, &box);

        SDL_Color white{250, 250, 255, 255};
        text_->draw(r, "Aperte E para entrar no portal",
                    px + 12, py + 12, white);
    }

    SDL_RenderPresent(r);
}
