#include "SoundManager.h"
#include <iostream>

// Ajuste aqui se seus áudios estiverem em outra pasta
static const char* AUDIO_BASE_PATH = "../assets/sounds/";

SoundManager::~SoundManager() {
    shutdown();
}

bool SoundManager::init(int frequency, Uint16 format, int channels, int chunksize) {
    if (initialized_) return true;

    // Inicializa os formatos que vamos usar
    int flags = MIX_INIT_OGG | MIX_INIT_MP3 | MIX_INIT_FLAC;
    int initted = Mix_Init(flags);
    if ((initted & flags) != flags) {
        std::cerr << "Mix_Init error: " << Mix_GetError() << "\n";
        // ainda assim tentamos continuar; algumas builds não têm todos os codecs
    }

    if (Mix_OpenAudio(frequency, format, channels, chunksize) < 0) {
        std::cerr << "Mix_OpenAudio error: " << Mix_GetError() << "\n";
        return false;
    }

    Mix_AllocateChannels(32); // 32 canais de SFX devem ser suficientes
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
    if (!chunk) {
        std::cerr << "Erro ao carregar SFX '" << id << "' (" << fullPath
                  << "): " << Mix_GetError() << "\n";
        return false;
    }
    // Se já existir, libera o antigo
    auto it = sfx_.find(id);
    if (it != sfx_.end() && it->second) {
        Mix_FreeChunk(it->second);
    }
    sfx_[id] = chunk;
    return true;
}

bool SoundManager::loadMusic(const std::string& id, const std::string& path) {
    std::string fullPath = std::string(AUDIO_BASE_PATH) + path;
    Mix_Music* mus = Mix_LoadMUS(fullPath.c_str());
    if (!mus) {
        std::cerr << "Erro ao carregar Music '" << id << "' (" << fullPath
                  << "): " << Mix_GetError() << "\n";
        return false;
    }
    auto it = music_.find(id);
    if (it != music_.end() && it->second) {
        Mix_FreeMusic(it->second);
    }
    music_[id] = mus;
    return true;
}

bool SoundManager::loadAll() {
    bool ok = true;

    // --------- MÚSICAS ---------
    ok = loadMusic("battle_theme",   "battle_theme.wav")   && ok;
    ok = loadMusic("menu_music",     "menu_music.wav")     && ok;
    ok = loadMusic("overworld_theme","overworld_theme.ogg")&& ok;
    ok = loadMusic("game_clear",     "jogo_zerado.wav")    && ok;

    // --------- SFX ---------
    ok = loadSfx("boss_attack",  "boss_hit.flac")      && ok; // boss causando dano
    ok = loadSfx("bow",          "bow.wav")            && ok; // ataques do caçador
    ok = loadSfx("click_button",        "click_button.wav")   && ok; // confirmar (enter)
    ok = loadSfx("enter_portal", "entrar_portal.ogg")  && ok;
    ok = loadSfx("step1",        "passos1.wav")        && ok;
    ok = loadSfx("step2",        "passos2.wav")        && ok;
    ok = loadSfx("recover_st",   "recuperar_st.aif")   && ok;
    ok = loadSfx("select_button",       "select_button.wav")  && ok; // mover cursor
    ok = loadSfx("spell",        "spell.wav")          && ok; // mago
    ok = loadSfx("sword",        "sword.wav")          && ok; // guerreiro
    ok = loadSfx("use_potion",   "usar_pocao.aif")     && ok; // poção

    return ok;
}

void SoundManager::playSfx(const std::string& id, int loops, int channel) {
    auto it = sfx_.find(id);
    if (it == sfx_.end() || !it->second) {
        // silencioso se não achar
        return;
    }
    Mix_PlayChannel(channel, it->second, loops);
}

void SoundManager::playMusic(const std::string& id, int loops) {
    auto it = music_.find(id);
    if (it == music_.end() || !it->second) {
        return;
    }

    // Se já estiver tocando outra, para com fade leve
    if (Mix_PlayingMusic()) {
        Mix_FadeOutMusic(300);
    }
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

    // Define volume padrão para todos os chunks carregados
    for (auto& kv : sfx_) {
        if (kv.second) Mix_VolumeChunk(kv.second, volume);
    }
}

void SoundManager::setMusicVolume(int volume) {
    if (volume < 0) volume = 0;
    if (volume > MIX_MAX_VOLUME) volume = MIX_MAX_VOLUME;
    Mix_VolumeMusic(volume);
}
