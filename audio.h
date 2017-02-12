#ifndef AUDIO_H_INCLUDED
#define AUDIO_H_INCLUDED

#include "base.h"
#include "clock.h"
#include "frame_queue.h"
#include "libswresample/swresample.h"
#include "libavutil/time.h"
#include "libavutil/avutil.h"

void update_sample_display(VideoState *is, short *samples, int samples_size);

int synchronize_audio(VideoState *is, int nb_samples);

int audio_decode_frame(VideoState *is);

void sdl_audio_callback(void *opaque, Uint8 *stream, int len);

int audio_open(void *opaque, int64_t wanted_channel_layout, int wanted_nb_channels, int wanted_sample_rate, struct AudioParams *audio_hw_params);

#endif // AUDIO_H_INCLUDED
