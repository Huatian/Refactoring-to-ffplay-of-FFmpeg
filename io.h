#ifndef IO_H_INCLUDED
#define IO_H_INCLUDED

#include "base.h"
#include "opt.h"
#include "packet_queue.h"
#include "frame_queue.h"
#include "decode.h"
#include "audio.h"
#include "video.h"
#include "cmdutils.h"
#include "libswresample/swresample.h"
#include "libswscale/swscale.h"
#include "libavutil/avstring.h"


#define MAX_QUEUE_SIZE (15 * 1024 * 1024)
#define MIN_FRAMES 25

/**
 * 打开指定的媒体流
 */
int stream_component_open(VideoState *is, int stream_index);

int decode_interrupt_cb(void *ctx);

int is_realtime(AVFormatContext *s);

/**
 * 开启源文件包含的媒体流
 * 开启解封装循环
 */
int read_thread(void *arg);

/**
 * 初始化大结构体、frame、packaet队列
 * 初始化时钟 clocks
 * 开启解封装线程 read_thread
 */
VideoState *stream_open(const char *filename, AVInputFormat *iformat);

void decoder_destroy(Decoder *d);

void decoder_abort(Decoder *d, FrameQueue *fq);

void stream_component_close(VideoState *is, int stream_index);

void stream_close(VideoState *is);

void do_exit(VideoState *is);

#endif // IO_H_INCLUDED
