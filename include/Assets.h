#pragma once
#include <string>
#include <SDL.h>

// Resolve caminho de asset baseado na estrutura do projeto
std::string resolveAsset(const std::string& rel);

// Carrega textura PNG com SDL_image
SDL_Texture* loadTexture(SDL_Renderer* r, const std::string& rel);
