#ifndef HANDLE_H_INCLUDED
#define HANDLE_H_INCLUDED

#include "base.h"
#include "clock.h"
#include "video.h"
#include "libavutil/time.h"

void sigterm_handler(int sig);

int lockmgr(void **mtx, enum AVLockOp op);

void stream_toggle_pause(VideoState *is);

void toggle_pause(VideoState *is);

void toggle_full_screen(VideoState *is);

void toggle_mute(VideoState *is);

void update_volume(VideoState *is, int sign, int step);

void step_to_next_frame(VideoState *is);

void stream_cycle_channel(VideoState *is, int codec_type);

void toggle_audio_display(VideoState *is);

void stream_seek(VideoState *is, int64_t pos, int64_t rel, int seek_by_bytes);

void seek_chapter(VideoState *is, int incr);

#endif // HANDLE_H_INCLUDED
