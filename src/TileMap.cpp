#include "TileMap.h"
#include "Assets.h"     // resolveAsset / loadTexture
#include <fstream>
#include <sstream>

TileMap::~TileMap() {
    if (tileset_) SDL_DestroyTexture(tileset_);
}

// Lê o CSV, usando resolveAsset para achar o arquivo
static bool readCSV(const std::string& rel, std::vector<int>& out, int& cols, int& rows) {
    std::string path = resolveAsset(rel);
    std::ifstream f(path);
    if (!f) return false;

    std::string line;
    rows = 0;
    cols = 0;
    out.clear();

    while (std::getline(f, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string cell;
        int colCount = 0;

        while (std::getline(ss, cell, ',')) {
            if (cell.empty()) continue;
            int v = std::stoi(cell);   // pode ser -1 ou 0..N-1
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
    if (tileset_) {
        SDL_DestroyTexture(tileset_);
        tileset_ = nullptr;
    }

    tileset_ = loadTexture(r, tilesetPath);
    if (!tileset_) return false;

    tileW_ = tileW;
    tileH_ = tileH;
    tilesetCols_ = tilesetCols;

    if (!readCSV(csvPath, data_, cols_, rows_)) return false;
    return true;
}

void TileMap::render(SDL_Renderer* r, int ox, int oy) const {
    if (!tileset_ || data_.empty()) return;

    SDL_Rect dst{0, 0, tileW_, tileH_};

    for (int y = 0; y < rows_; ++y) {
        for (int x = 0; x < cols_; ++x) {
            int id = data_[y * cols_ + x];   // ID vindo do CSV

            // -1 = vazio (não desenha)
            if (id < 0) continue;

            // IDs já são 0-based: 0..N-1, índice direto no tileset
            int idx = id;

            int sx = (idx % tilesetCols_) * tileW_;
            int sy = (idx / tilesetCols_) * tileH_;
            SDL_Rect src{ sx, sy, tileW_, tileH_ };

            dst.x = ox + x * tileW_;
            dst.y = oy + y * tileH_;

            SDL_RenderCopy(r, tileset_, &src, &dst);
        }
    }
}

// -----------------------------------------------------------
// Acesso / colisão
// -----------------------------------------------------------

int TileMap::tileIdAt(int tx, int ty) const {
    if (tx < 0 || ty < 0 || tx >= cols_ || ty >= rows_) {
        return -1; // fora do mapa = vazio
    }
    int id = data_[ty * cols_ + tx];
    return id;     // pode ser -1 (vazio) ou 0..N-1
}

int TileMap::tileIdAtPixel(int px, int py) const {
    if (tileW_ <= 0 || tileH_ <= 0) return -1;

    int tx = px / tileW_;
    int ty = py / tileH_;
    return tileIdAt(tx, ty);
}

// IDs sólidos: 0,1,2,3,8,10,16,17,18,19
bool TileMap::isSolidId(int id) const {
    if (id < 0) return false; // vazio nunca é sólido

    switch (id) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 8:
        case 10:
        case 16:
        case 17:
        case 18:
        case 19:
            return true;
        default:
            return false;
    }
}

bool TileMap::isSolidAt(int tx, int ty) const {
    int id = tileIdAt(tx, ty);
    return isSolidId(id);
}

bool TileMap::isSolidAtPixel(int px, int py) const {
    int id = tileIdAtPixel(px, py);
    return isSolidId(id);
}
