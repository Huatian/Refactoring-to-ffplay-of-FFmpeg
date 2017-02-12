#ifndef CLOCK_H_INCLUDED
#define CLOCK_H_INCLUDED

#include "base.h"
#include "libavutil/time.h"

/* no AV sync correction is done if below the minimum AV sync threshold */
#define AV_SYNC_THRESHOLD_MIN 0.04
/* AV sync correction is done if above the maximum AV sync threshold */
#define AV_SYNC_THRESHOLD_MAX 0.1
/* If a frame duration is longer than this, it will not be duplicated to compensate AV sync */
#define AV_SYNC_FRAMEDUP_THRESHOLD 0.1
/* no AV correction is done if too big error */
#define AV_NOSYNC_THRESHOLD 10.0

/* maximum audio speed change to get correct sync */
#define SAMPLE_CORRECTION_PERCENT_MAX 10

/* external clock speed adjustment constants for realtime sources based on buffer fullness */
#define EXTERNAL_CLOCK_SPEED_MIN  0.900
#define EXTERNAL_CLOCK_SPEED_MAX  1.010
#define EXTERNAL_CLOCK_SPEED_STEP 0.001

/* we use about AUDIO_DIFF_AVG_NB A-V differences to make the average */
#define AUDIO_DIFF_AVG_NB   20

#define EXTERNAL_CLOCK_MIN_FRAMES 2
#define EXTERNAL_CLOCK_MAX_FRAMES 10

double get_clock(Clock *c);

void set_clock_at(Clock *c, double pts, int serial, double time);

void set_clock(Clock *c, double pts, int serial);

void set_clock_speed(Clock *c, double speed);

void check_external_clock_speed(VideoState *is);

void init_clock(Clock *c, int *queue_serial);

void sync_clock_to_slave(Clock *c, Clock *slave);

int get_master_sync_type(VideoState *is);

double get_master_clock(VideoState *is);

double compute_target_delay(double delay, VideoState *is);

#endif // CLOCK_H_INCLUDED
