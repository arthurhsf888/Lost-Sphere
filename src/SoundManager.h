#pragma once
#include <string>
#include <unordered_map>
#include <SDL_mixer.h>

class SoundManager {
public:
    SoundManager() = default;
    ~SoundManager();

    bool init(int frequency = 44100,
              Uint16 format = MIX_DEFAULT_FORMAT,
              int channels = 2,
              int chunksize = 1024);

    void shutdown();
    bool loadAll();
    bool loadSfx(const std::string& id, const std::string& path);
    bool loadMusic(const std::string& id, const std::string& path);

    void playSfx(const std::string& id, int loops = 0, int channel = -1);
    void playMusic(const std::string& id, int loops = -1);

    void stopMusic();
    void fadeOutMusic(int ms);

    void setSfxVolume(int volume);
    void setMusicVolume(int volume);

    bool ok() const { return initialized_; }

private:
    bool initialized_ = false;

    std::unordered_map<std::string, Mix_Chunk*> sfx_;
    std::unordered_map<std::string, Mix_Music*> music_;

    void freeAll();
};
