#pragma once
#include <SDL.h>
#include <string>
#include <vector>

class TileMap {
public:
    ~TileMap();

    bool load(SDL_Renderer* r,
              const std::string& tilesetPath,
              const std::string& csvPath,
              int tileW, int tileH,
              int tilesetCols);

    void render(SDL_Renderer* r, int ox = 0, int oy = 0) const;

    int cols() const { return cols_; }
    int rows() const { return rows_; }
    int tileW() const { return tileW_; }
    int tileH() const { return tileH_; }

    int tileIdAt(int tx, int ty) const;
    int tileIdAtPixel(int px, int py) const;
    bool isSolidId(int id) const;
    bool isSolidAt(int tx, int ty) const;
    bool isSolidAtPixel(int px, int py) const;

private:
    SDL_Texture* tileset_ = nullptr;
    int tileW_ = 32, tileH_ = 32;
    int tilesetCols_ = 1;

    int cols_ = 0, rows_ = 0;
    std::vector<int> data_;
};
