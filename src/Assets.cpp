#include "../include/Assets.h"
#include <SDL_image.h>
#include <filesystem>
#include <vector>

std::string resolveAsset(const std::string& rel) {
    std::vector<std::string> cand;

    // 1) relativo ao diretório do executável
    char* baseC = SDL_GetBasePath();
    std::string base = baseC ? baseC : "";
    if (baseC) SDL_free(baseC);
    if (!base.empty()) {
        cand.push_back(base + rel);
        cand.push_back(base + "../" + rel);
        cand.push_back(base + "../../" + rel);
    }

    // 2) relativo ao diretório de trabalho
    cand.push_back(rel);

    for (auto& p : cand) {
        if (std::filesystem::exists(p)) return p;
    }
    return rel; // deixa falhar, mas pelo menos sabemos o que tentou
}

SDL_Texture* loadTexture(SDL_Renderer* r, const std::string& rel) {
    std::string path = resolveAsset(rel);
    SDL_Texture* tex = IMG_LoadTexture(r, path.c_str());
    if (!tex) {
        SDL_Log("IMG_LoadTexture falhou para %s: %s", path.c_str(), IMG_GetError());
    }
    return tex;
}
