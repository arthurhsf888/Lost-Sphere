#include "TileMap.h"
#include "Assets.h"     // usa loadTexture(...)
#include <fstream>
#include <sstream>

TileMap::~TileMap() {
    if (tileset_) SDL_DestroyTexture(tileset_);
}

static bool readCSV(const std::string& path, std::vector<int>& out, int& cols, int& rows) {
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
            int v = std::stoi(cell);
            out.push_back(v);
            colCount++;
        }
        if (cols == 0) cols = colCount;
        rows++;
    }
    return (cols > 0 && rows > 0);
}

bool TileMap::load(SDL_Renderer* r, const std::string& tilesetPath,
                   const std::string& csvPath, int tileW, int tileH, int tilesetCols) {
    if (tileset_) { SDL_DestroyTexture(tileset_); tileset_ = nullptr; }
    tileset_ = loadTexture(r, tilesetPath);
    if (!tileset_) return false;

    tileW_ = tileW; tileH_ = tileH; tilesetCols_ = tilesetCols;

    if (!readCSV(csvPath, data_, cols_, rows_)) return false;
    return true;
}

void TileMap::render(SDL_Renderer* r, int ox, int oy) const {
    if (!tileset_ || data_.empty()) return;

    SDL_Rect dst{0,0,tileW_,tileH_};
    for (int y = 0; y < rows_; ++y) {
        for (int x = 0; x < cols_; ++x) {
            int id = data_[y*cols_ + x];     // id do Tiled
            if (id <= 0) continue;           // 0 = vazio
            int idx = id - 1;                // Tiled começa em 1
            int sx = (idx % tilesetCols_) * tileW_;
            int sy = (idx / tilesetCols_) * tileH_;
            SDL_Rect src{ sx, sy, tileW_, tileH_ };

            dst.x = ox + x*tileW_;
            dst.y = oy + y*tileH_;
            SDL_RenderCopy(r, tileset_, &src, &dst);
        }
    }
}
