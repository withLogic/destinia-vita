#include "audio.h"
#include "soloud.h"
#include "soloud_wav.h"
#include "soloud_wavstream.h"
#include <mutex>
#include <map>
#include <stdio.h>
#include <sys/stat.h>
#include "utils/logger.h"
#include "minimp3.h"
#include "minimp3_ext.h"

static SoLoud::Soloud gSoloud;

static std::map<int, SoLoud::AudioSource*> gAudioCache;
static std::mutex gAudioMutex;
static SoLoud::handle gMusicHandle = 0;
static bool gAudioInit = false;
static int gAudioCurrentBgTrack = -1;

void audio_init() {
    std::lock_guard<std::mutex> lock(gAudioMutex);
    SoLoud::result r = gSoloud.init();
    if (r != SoLoud::SO_NO_ERROR) {
        l_debug("[audio_init] gSoloud.init() failed: %d", r);
        return;
    }
    gAudioInit = true;
}

void audio_cleanup() {
    std::lock_guard<std::mutex> lock(gAudioMutex);
    if(!gAudioInit){
        return;
    }

    gSoloud.stopAll();

    for (auto const& [id, source] : gAudioCache) {
        delete source;
    }
    gAudioCache.clear();

    gSoloud.deinit();
}

SoLoud::AudioSource* audio_mp3_load(int sndID) {
    char cachePath[256];
    snprintf(cachePath, sizeof(cachePath), DATA_PATH "res/raw/bg%03d.wav", sndID);

    FILE* wavFile = fopen(cachePath, "rb");
    if (wavFile) {
        fclose(wavFile);
        SoLoud::Wav* music = new SoLoud::Wav();
        music->load(cachePath);
        return music;
    }

    char filepath[256];
    snprintf(filepath, sizeof(filepath), DATA_PATH "res/raw/bg%03d.mp3", sndID);

    mp3dec_t mp3d;
    mp3dec_file_info_t info;

    if (mp3dec_load(&mp3d, filepath, &info, NULL, NULL)) {
        l_debug("[audio_mp3_load] mp3dec_load failed for %s", filepath);
        return nullptr;
    }

    unsigned int channels = info.channels;
    unsigned int frameCount = info.samples / channels;

    float* planar = new float[frameCount * channels];
    for (unsigned int frame = 0; frame < frameCount; frame++) {
        for (unsigned int ch = 0; ch < channels; ch++) {
            planar[ch * frameCount + frame] = info.buffer[frame * channels + ch] / 32768.0f;
        }
    }

    free(info.buffer);

    audio_save_wav(cachePath, planar, frameCount, channels, info.hz);

    SoLoud::Wav* music = new SoLoud::Wav();
    music->mData = planar;
    music->mSampleCount = frameCount;
    music->mChannels = channels;
    music->mBaseSamplerate = (float)info.hz;

    return music;
}

void audio_preload(int sndID){
    if(sndID != -1 && sndID < 100){
        return;
    }

    if(sndID == -1){
        for(int i = 100; i < 108; i++){
            l_debug("[audio_preload] preloading sndID=%d", i);
             gAudioCache[i] = audio_mp3_load(i);
        }
    } else {
        l_debug("[audio_preload] preloading sndID=%d", sndID);
         gAudioCache[sndID] = audio_mp3_load(sndID);
    }
}

void audio_play_sound(int sndID) {
    
    std::lock_guard<std::mutex> lock(gAudioMutex);
    int isLoop = 0;
    int vol = 100;

    if (sndID < 100) {
        isLoop = 0;
    } else {
        isLoop = 1;
        if(gAudioCurrentBgTrack == sndID){
            return;
        } else {
            gAudioCurrentBgTrack = sndID;
        }        
    }

    l_debug("[audio_play_sound] sndID %d", sndID);

    if (gAudioCache.find(sndID) == gAudioCache.end()) {
        char filepath[256];
        if (sndID < 100) {
            snprintf(filepath, sizeof(filepath), DATA_PATH "res/raw/eff%02d.ogg", sndID);
        } else {
            snprintf(filepath, sizeof(filepath), DATA_PATH "res/raw/bg%03d.mp3", sndID);
        }

        l_debug("[audio_play_sound] filepath %s", filepath);

        SoLoud::AudioSource* newSource = nullptr;

        if (sndID < 100) {
            SoLoud::Wav* sfx = new SoLoud::Wav();
            sfx->load(filepath);
            newSource = sfx;
        } else {
            newSource = audio_mp3_load(sndID);
        }

        gAudioCache[sndID] = newSource;
    }

    SoLoud::AudioSource* sourceToPlay = gAudioCache[sndID];
    if (sourceToPlay != nullptr) {
        if (sndID >= 100) {
            if (gMusicHandle != 0) {
                gSoloud.stop(gMusicHandle);
            }
            gMusicHandle = gSoloud.play(*sourceToPlay);
            gSoloud.setProtectVoice(gMusicHandle, true);
            gSoloud.setVolume(gMusicHandle, (float)vol / 100.0f);
            gSoloud.setLooping(gMusicHandle, isLoop);
        } else {
            SoLoud::handle handle = gSoloud.play(*sourceToPlay);
            gSoloud.setVolume(handle, (float)vol / 100.0f);
            gSoloud.setLooping(handle, isLoop);
        }

    }
}

void audio_stop_sound() {
    std::lock_guard<std::mutex> lock(gAudioMutex);
    l_debug("[audio_check] called");
    gSoloud.stopAll();
}

void audio_debug(){
    l_debug("[audio_check] gMusicHandle=%u valid=%d", gMusicHandle, gSoloud.isValidVoiceHandle(gMusicHandle));
}

void audio_save_wav(const char* path, const float* planar, unsigned int frameCount, unsigned int channels, unsigned int sampleRate) {

    FILE* saveFile = fopen(path, "wb");
    if (!saveFile) {
        l_debug("[audio_save_wav] Failed to open %s for writing", path);
        return;
    }

    uint32_t dataSize = frameCount * channels * sizeof(int16_t);
    uint32_t byteRate = sampleRate * channels * sizeof(int16_t);
    uint16_t blockAlign = channels * sizeof(int16_t);
    uint32_t chunkSize = 36 + dataSize;

    fwrite("RIFF", 1, 4, saveFile);
    fwrite(&chunkSize, 4, 1, saveFile);
    fwrite("WAVE", 1, 4, saveFile);

    fwrite("fmt ", 1, 4, saveFile);
    uint32_t fmtSize = 16;
    uint16_t audioFormat = 1;
    uint16_t numChannels = (uint16_t)channels;
    uint16_t bitsPerSample = 16;
    fwrite(&fmtSize, 4, 1, saveFile);
    fwrite(&audioFormat, 2, 1, saveFile);
    fwrite(&numChannels, 2, 1, saveFile);
    fwrite(&sampleRate, 4, 1, saveFile);
    fwrite(&byteRate, 4, 1, saveFile);
    fwrite(&blockAlign, 2, 1, saveFile);
    fwrite(&bitsPerSample, 2, 1, saveFile);

    fwrite("data", 1, 4, saveFile);
    fwrite(&dataSize, 4, 1, saveFile);

    int16_t* interleaved = new int16_t[frameCount * channels];
    for (unsigned int frame = 0; frame < frameCount; frame++) {
        for (unsigned int ch = 0; ch < channels; ch++) {
            float sample = planar[ch * frameCount + frame];
            if (sample > 1.0f) sample = 1.0f;
            if (sample < -1.0f) sample = -1.0f;
            interleaved[frame * channels + ch] = (int16_t)(sample * 32767.0f);
        }
    }
    fwrite(interleaved, sizeof(int16_t), frameCount * channels, saveFile);
    delete[] interleaved;

    fclose(saveFile);
    l_debug("[audio_save_wav] Wrote %s (%u frames, %u ch, %u hz)", path, frameCount, channels, sampleRate);
}