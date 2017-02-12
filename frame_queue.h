#ifndef FRAME_QUEUE_H_INCLUDED
#define FRAME_QUEUE_H_INCLUDED

#include "base.h"

void frame_queue_unref_item(Frame *vp);

int frame_queue_init(FrameQueue *f, PacketQueue *pktq, int max_size, int keep_last);

void frame_queue_destory(FrameQueue *f);

void frame_queue_signal(FrameQueue *f);

Frame *frame_queue_peek(FrameQueue *f);

Frame *frame_queue_peek_next(FrameQueue *f);

Frame *frame_queue_peek_last(FrameQueue *f);

/**
 * 获取可写入的位置
 */
Frame *frame_queue_peek_writable(FrameQueue *f);

Frame *frame_queue_peek_readable(FrameQueue *f);

/**
 * 更新写入指针和size;放在frame_queue_peek_writable后搭配使用
 */
void frame_queue_push(FrameQueue *f);

/**
 * frame_queue_peek_readable 之后的操作
 * 释放当前读取指针指向的Frame
 * 将读取指针向前、size--
 */
void frame_queue_next(FrameQueue *f);

/**
 * 当 force_refresh 时，跳回上一帧并重置rindex_shown
 */
int frame_queue_prev(FrameQueue *f);

/**
 * 返回队列中没显示的帧数
 */
int frame_queue_nb_remaining(FrameQueue *f);

/**
 * 获取刚播放的一帧在源文件中的位置（按byte计算）
 */
int64_t frame_queue_last_pos(FrameQueue *f);

#endif // FRAME_QUEUE_H_INCLUDED
