#ifndef DECODE_H_INCLUDED
#define DECODE_H_INCLUDED

#include "base.h"
#include "opt.h"
#include "clock.h"
#include "packet_queue.h"
#include "frame_queue.h"
#include "libavutil/imgutils.h"
#include "libavutil/opt.h"

void decoder_init(Decoder *d, AVCodecContext *avctx, PacketQueue *queue, SDL_cond *empty_queue_cond);

/* 完成视频流、音频流、字幕流的解码工作 */
int decoder_decode_frame(Decoder *d, AVFrame *frame, AVSubtitle *sub);

int subtitle_thread(void *arg);

int audio_thread(void *arg);

int get_video_frame(VideoState *is, AVFrame *frame);

void duplicate_right_border_pixels(SDL_Overlay *bmp);

int queue_picture(VideoState *is, AVFrame *src_frame, double pts, double duration, int64_t pos, int serial);

int video_thread(void *arg);

int decoder_start(Decoder *d, int (*fn)(void *), void *arg);

#endif // DECODE_H_INCLUDED
