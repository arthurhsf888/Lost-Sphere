#pragma once
#include <SDL.h>
#include <string>
#include <vector>

class TileMap {
public:
    ~TileMap();

    // tilesetPath: PNG com o tileset
    // csvPath: mapa exportado como CSV pelo Tiled
    // tileW/H: tamanho de cada tile (ex.: 32x32)
    // tilesetCols: quantas colunas tem o tileset (para calcular o src)
    bool load(SDL_Renderer* r, const std::string& tilesetPath,
              const std::string& csvPath, int tileW, int tileH, int tilesetCols);

    void render(SDL_Renderer* r, int ox = 0, int oy = 0) const; // offset opcional

    int cols() const { return cols_; }
    int rows() const { return rows_; }
    int tileW() const { return tileW_; }
    int tileH() const { return tileH_; }

private:
    SDL_Texture* tileset_ = nullptr;
    int tileW_ = 32, tileH_ = 32;
    int tilesetCols_ = 1;

    int cols_ = 0, rows_ = 0;            // tamanho do mapa (em tiles)
    std::vector<int> data_;              // ids do Tiled (1..N) ou 0 para vazio
};
