#pragma once
#include <SDL.h>
#include <string>
#include <vector>

class TileMap {
public:
    ~TileMap();

    // tilesetPath: PNG com o tileset
    // csvPath: mapa exportado como CSV
    //  - IDs no CSV são 0-based (0,1,2,...), seguindo a posição no tileset.
    //  - Use -1 para "vazio" (sem tile).
    // tileW/H: tamanho de cada tile (ex.: 32x32)
    // tilesetCols: quantas colunas tem o tileset (para calcular o src)
    bool load(SDL_Renderer* r, const std::string& tilesetPath,
              const std::string& csvPath, int tileW, int tileH, int tilesetCols);

    // Desenha o mapa com offset opcional (ox, oy)
    void render(SDL_Renderer* r, int ox = 0, int oy = 0) const;

    // --- Infos básicas do mapa ---
    int cols() const { return cols_; }
    int rows() const { return rows_; }
    int tileW() const { return tileW_; }
    int tileH() const { return tileH_; }

    // --- Acesso a tiles / colisão ---

    // ID cru do tile nas coordenadas de grade (tx,ty).
    // Retorna -1 se estiver fora do mapa ou se o valor for -1.
    int tileIdAt(int tx, int ty) const;

    // ID do tile na posição em pixels (px,py).
    // Converte para (tx,ty) usando tileW_/tileH_.
    int tileIdAtPixel(int px, int py) const;

    // Retorna true se o ID for considerado sólido (parede).
    bool isSolidId(int id) const;

    // Retorna true se o tile em (tx,ty) for sólido.
    bool isSolidAt(int tx, int ty) const;

    // Retorna true se o ponto em pixels (px,py) estiver em um tile sólido.
    bool isSolidAtPixel(int px, int py) const;

private:
    SDL_Texture* tileset_ = nullptr;
    int tileW_ = 32, tileH_ = 32;
    int tilesetCols_ = 1;

    int cols_ = 0, rows_ = 0;   // tamanho do mapa (em tiles)
    // IDs 0..N-1 (índice direto no tileset), -1 = vazio.
    std::vector<int> data_;
};
