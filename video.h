#ifndef VIDEO_H_INCLUDED
#define VIDEO_H_INCLUDED

#include "base.h"
#include "opt.h"
#include "io.h"
#include "handle.h"
#include "frame_queue.h"
#include "libavutil/time.h"

void alloc_picture(VideoState *is);

void fill_rectangle(SDL_Surface *screen,
                                  int x, int y, int w, int h, int color, int update);

void fill_border(int xleft, int ytop, int width, int height, int x, int y, int w, int h, int color, int update);

void blend_subrect(uint8_t **data, int *linesize, const AVSubtitleRect *rect, int imgw, int imgh);

void calculate_display_rect(SDL_Rect *rect,
                                   int scr_xleft, int scr_ytop, int scr_width, int scr_height,
                                   int pic_width, int pic_height, AVRational pic_sar);

void video_image_display(VideoState *is);

void set_default_window_size(int width, int height, AVRational sar);

int video_open(VideoState *is, int force_set_video_mode, Frame *vp);

int compute_mod(int a, int b);

/**
 * 视频不显示，只显示波形图
 */
void video_audio_display(VideoState *s);

/**
 * 显示当前的图片
 */
void video_display(VideoState *is);

double vp_duration(VideoState *is, Frame *vp, Frame *nextvp);

void update_video_pts(VideoState *is, double pts, int64_t pos, int serial);

/**
 * 刷新视频，包括处理音视频同步
 */
void video_refresh(void *opaque, double *remaining_time);

#endif // VIDEO_H_INCLUDED
