#ifndef PACKET_QUEUE_H_INCLUDED
#define PACKET_QUEUE_H_INCLUDED

#include "base.h"

AVPacket flush_pkt;

int packet_queue_put_private(PacketQueue *q, AVPacket *pkt);

int packet_queue_put(PacketQueue *q, AVPacket *pkt);

int packet_queue_put_nullpacket(PacketQueue *q, int stream_index);

int packet_queue_init(PacketQueue *q);

/**
 * 清空packet queue，保留序列号
 */
void packet_queue_flush(PacketQueue *q);

void packet_queue_destroy(PacketQueue *q);

void packet_queue_abort(PacketQueue *q);

void packet_queue_start(PacketQueue *q);

int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block, int *serial);



#endif // PACKET_QUEUE_H_INCLUDED
