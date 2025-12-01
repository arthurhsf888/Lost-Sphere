#include "SoundManager.h"
#include <iostream>

static const char* AUDIO_BASE_PATH = "../assets/sounds/";

SoundManager::~SoundManager() {
    shutdown();
}

bool SoundManager::init(int frequency, Uint16 format, int channels, int chunksize) {
    if (initialized_) return true;

    int flags = MIX_INIT_OGG | MIX_INIT_MP3 | MIX_INIT_FLAC;
    int initted = Mix_Init(flags);

    Mix_OpenAudio(frequency, format, channels, chunksize);
    Mix_AllocateChannels(32);

    initialized_ = true;
    return true;
}

void SoundManager::shutdown() {
    if (!initialized_) return;
    freeAll();
    Mix_CloseAudio();
    Mix_Quit();
    initialized_ = false;
}

void SoundManager::freeAll() {
    for (auto& kv : sfx_) {
        if (kv.second) Mix_FreeChunk(kv.second);
    }
    sfx_.clear();

    for (auto& kv : music_) {
        if (kv.second) Mix_FreeMusic(kv.second);
    }
    music_.clear();
}

bool SoundManager::loadSfx(const std::string& id, const std::string& path) {
    std::string fullPath = std::string(AUDIO_BASE_PATH) + path;
    Mix_Chunk* chunk = Mix_LoadWAV(fullPath.c_str());
    if (!chunk) return false;

    auto it = sfx_.find(id);
    if (it != sfx_.end() && it->second) Mix_FreeChunk(it->second);
    sfx_[id] = chunk;
    return true;
}

bool SoundManager::loadMusic(const std::string& id, const std::string& path) {
    std::string fullPath = std::string(AUDIO_BASE_PATH) + path;
    Mix_Music* mus = Mix_LoadMUS(fullPath.c_str());
    if (!mus) return false;

    auto it = music_.find(id);
    if (it != music_.end() && it->second) Mix_FreeMusic(it->second);
    music_[id] = mus;
    return true;
}

bool SoundManager::loadAll() {
    bool ok = true;

    ok = loadMusic("battle_theme",   "battle_theme.wav")    && ok;
    ok = loadMusic("menu_music",     "menu_music.wav")      && ok;
    ok = loadMusic("overworld_theme","overworld_theme.ogg") && ok;
    ok = loadMusic("game_clear",     "jogo_zerado.wav")     && ok;

    ok = loadSfx("boss_attack",  "boss_hit.flac")      && ok;
    ok = loadSfx("bow",          "bow.wav")            && ok;
    ok = loadSfx("click_button", "click_button.wav")    && ok;
    ok = loadSfx("enter_portal", "entrar_portal.ogg")  && ok;
    ok = loadSfx("step1",        "passos1.wav")        && ok;
    ok = loadSfx("step2",        "passos2.wav")        && ok;
    ok = loadSfx("recover_st",   "recuperar_st.aif")   && ok;
    ok = loadSfx("select_button","select_button.wav")  && ok;
    ok = loadSfx("spell",        "spell.wav")          && ok;
    ok = loadSfx("sword",        "sword.wav")          && ok;
    ok = loadSfx("use_potion",   "usar_pocao.aif")     && ok;

    return ok;
}

void SoundManager::playSfx(const std::string& id, int loops, int channel) {
    auto it = sfx_.find(id);
    if (it == sfx_.end() || !it->second) return;
    Mix_PlayChannel(channel, it->second, loops);
}

void SoundManager::playMusic(const std::string& id, int loops) {
    auto it = music_.find(id);
    if (it == music_.end() || !it->second) return;

    if (Mix_PlayingMusic()) Mix_FadeOutMusic(300);
    Mix_PlayMusic(it->second, loops);
}

void SoundManager::stopMusic() {
    Mix_HaltMusic();
}

void SoundManager::fadeOutMusic(int ms) {
    Mix_FadeOutMusic(ms);
}

void SoundManager::setSfxVolume(int volume) {
    if (volume < 0) volume = 0;
    if (volume > MIX_MAX_VOLUME) volume = MIX_MAX_VOLUME;

    for (auto& kv : sfx_) {
        if (kv.second) Mix_VolumeChunk(kv.second, volume);
    }
}

void SoundManager::setMusicVolume(int volume) {
    if (volume < 0) volume = 0;
    if (volume > MIX_MAX_VOLUME) volume = MIX_MAX_VOLUME;
    Mix_VolumeMusic(volume);
}
