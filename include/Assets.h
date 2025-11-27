#pragma once
#include <string>
#include <SDL.h>

std::string resolveAsset(const std::string& rel);

// carrega textura PNG usando SDL_image; retorna nullptr se falhar
SDL_Texture* loadTexture(SDL_Renderer* r, const std::string& rel);
