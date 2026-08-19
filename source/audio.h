#ifndef AUDIO_H
#define AUDIO_H

#include "utils/logger.h"

#ifdef __cplusplus
extern "C"
{
#endif

void audio_init();
void audio_cleanup();
void audio_play_sound(int sndID);
void audio_stop_sound();
void audio_debug();
void audio_preload(int sndID);
void audio_save_wav(const char* path, const float* planar, unsigned int frameCount, unsigned int channels, unsigned int sampleRate);

#ifdef __cplusplus
}
#endif

#endif // AUDIO_H