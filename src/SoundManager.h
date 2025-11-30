#pragma once
#include <string>
#include <unordered_map>
#include <SDL_mixer.h>

// Gerencia efeitos sonoros (Mix_Chunk) e músicas (Mix_Music).
// Uso típico:
//   SoundManager snd;
//   snd.init();
//   snd.loadAll();
//   snd.playMusic("overworld_theme", -1);
//   snd.playSfx("select");
class SoundManager {
public:
    SoundManager() = default;
    ~SoundManager();

    // Inicializa SDL_mixer (deve ser chamado depois de SDL_Init)
    bool init(int frequency = 44100,
              Uint16 format = MIX_DEFAULT_FORMAT,
              int channels = 2,
              int chunksize = 1024);

    // Libera tudo e fecha o mixer
    void shutdown();

    // Carrega todos os arquivos conhecidos (mapeados no .cpp)
    bool loadAll();

    // Carrega um efeito sonoro individual (id -> caminho)
    bool loadSfx(const std::string& id, const std::string& path);

    // Carrega uma música individual (id -> caminho)
    bool loadMusic(const std::string& id, const std::string& path);

    // Toca um efeito sonoro. loops = 0 toca uma vez, -1 loop infinito.
    // channel = -1 deixa o mixer escolher.
    void playSfx(const std::string& id, int loops = 0, int channel = -1);

    // Toca música. loops = -1 para loop infinito.
    void playMusic(const std::string& id, int loops = -1);

    // Para música imediatamente ou com fade out
    void stopMusic();
    void fadeOutMusic(int ms);

    // Volumes: 0–128 (SDL_mixer)
    void setSfxVolume(int volume);   // afeta todos os chunks já carregados
    void setMusicVolume(int volume); // afeta música atual / futuras

    // Acesso simples para saber se tudo deu certo
    bool ok() const { return initialized_; }

private:
    bool initialized_ = false;

    std::unordered_map<std::string, Mix_Chunk*> sfx_;
    std::unordered_map<std::string, Mix_Music*> music_;

    void freeAll();
};
