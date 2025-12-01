#include "TileMap.h"
#include "Assets.h"
#include <fstream>
#include <sstream>

TileMap::~TileMap() {
    if (tileset_) SDL_DestroyTexture(tileset_);
}

static bool readCSV(const std::string& rel, std::vector<int>& out, int& cols, int& rows) {
    std::string path = resolveAsset(rel);
    std::ifstream f(path);
    if (!f) return false;

    std::string line;
    rows = 0; cols = 0;
    out.clear();

    while (std::getline(f, line)) {
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string cell;
        int colCount = 0;

        while (std::getline(ss, cell, ',')) {
            if (cell.empty()) continue;
            out.push_back(std::stoi(cell));
            colCount++;
        }

        if (cols == 0) cols = colCount;
        rows++;
    }
    return cols > 0 && rows > 0;
}

bool TileMap::load(SDL_Renderer* r, const std::string& tilesetPath,
                   const std::string& csvPath, int tileW, int tileH, int tilesetCols) {
    if (tileset_) SDL_DestroyTexture(tileset_);

    tileset_ = loadTexture(r, tilesetPath);
    if (!tileset_) return false;

    tileW_ = tileW;
    tileH_ = tileH;
    tilesetCols_ = tilesetCols;

    return readCSV(csvPath, data_, cols_, rows_);
}

void TileMap::render(SDL_Renderer* r, int ox, int oy) const {
    if (!tileset_ || data_.empty()) return;

    SDL_Rect dst{0, 0, tileW_, tileH_};

    for (int y = 0; y < rows_; ++y) {
        for (int x = 0; x < cols_; ++x) {
            int id = data_[y * cols_ + x];
            if (id < 0) continue;

            int sx = (id % tilesetCols_) * tileW_;
            int sy = (id / tilesetCols_) * tileH_;
            SDL_Rect src{ sx, sy, tileW_, tileH_ };

            dst.x = ox + x * tileW_;
            dst.y = oy + y * tileH_;

            SDL_RenderCopy(r, tileset_, &src, &dst);
        }
    }
}

int TileMap::tileIdAt(int tx, int ty) const {
    if (tx < 0 || ty < 0 || tx >= cols_ || ty >= rows_) return -1;
    return data_[ty * cols_ + tx];
}

int TileMap::tileIdAtPixel(int px, int py) const {
    int tx = px / tileW_;
    int ty = py / tileH_;
    return tileIdAt(tx, ty);
}

bool TileMap::isSolidId(int id) const {
    switch (id) {
        case 0: case 1: case 2: case 3:
        case 8: case 10:
        case 16: case 17: case 18: case 19:
            return true;
        default:
            return false;
    }
}

bool TileMap::isSolidAt(int tx, int ty) const {
    return isSolidId(tileIdAt(tx, ty));
}

bool TileMap::isSolidAtPixel(int px, int py) const {
    return isSolidId(tileIdAtPixel(px, py));
}
