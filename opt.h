#ifndef OPT_H_INCLUDED
#define OPT_H_INCLUDED

#include <libio.h>
#include "base.h"
#include "libavutil/log.h"
#include "libavutil/opt.h"
#include "libavformat/avformat.h"
#include "cmdutils.h"

/* options specified by the user */
AVInputFormat *file_iformat;
char *input_filename;
const char *window_title;
int fs_screen_width;/* 全屏宽度 */
int fs_screen_height;/* 全屏高度 */
int default_width;
int default_height;
int screen_width;
int screen_height;
int display_disable;
int audio_disable;
int video_disable;
int subtitle_disable;
const char* wanted_stream_spec[AVMEDIA_TYPE_NB];
int seek_by_bytes;
int show_status;
int av_sync_type;
int64_t start_time;
int64_t duration;
int fast;
int genpts;
int lowres;
int decoder_reorder_pts;
int autoexit;
int exit_on_keydown;
int exit_on_mousedown;
int loop;
int framedrop;
int infinite_buffer;
enum ShowMode show_mode;
const char *audio_codec_name;
const char *subtitle_codec_name;
const char *video_codec_name;
double rdftspeed;
int64_t cursor_last_shown;
int cursor_hidden;

int autorotate;

int opt_frame_size(void *optctx, const char *opt, const char *arg);

int opt_width(void *optctx, const char *opt, const char *arg);

int opt_height(void *optctx, const char *opt, const char *arg);

int opt_format(void *optctx, const char *opt, const char *arg);

int opt_frame_pix_fmt(void *optctx, const char *opt, const char *arg);

int opt_sync(void *optctx, const char *opt, const char *arg);

int opt_seek(void *optctx, const char *opt, const char *arg);

int opt_duration(void *optctx, const char *opt, const char *arg);

int opt_show_mode(void *optctx, const char *opt, const char *arg);

void opt_input_file(void *optctx, const char *filename);

int opt_codec(void *optctx, const char *opt, const char *arg);

int opt_add_vfilter(void *optctx, const char *opt, const char *arg);

void show_usage(void);

void show_help_default(const char *opt, const char *arg);

static const OptionDef options[] = {
#include "cmdutils_common_opts.h"
    { "x", HAS_ARG, { .func_arg = opt_width }, "force displayed width", "width" },
    { "y", HAS_ARG, { .func_arg = opt_height }, "force displayed height", "height" },
    { "s", HAS_ARG | OPT_VIDEO, { .func_arg = opt_frame_size }, "set frame size (WxH or abbreviation)", "size" },
    { "fs", OPT_BOOL, { &is_full_screen }, "force full screen" },
    { "an", OPT_BOOL, { &audio_disable }, "disable audio" },
    { "vn", OPT_BOOL, { &video_disable }, "disable video" },
    { "sn", OPT_BOOL, { &subtitle_disable }, "disable subtitling" },
    { "ast", OPT_STRING | HAS_ARG | OPT_EXPERT, { &wanted_stream_spec[AVMEDIA_TYPE_AUDIO] }, "select desired audio stream", "stream_specifier" },
    { "vst", OPT_STRING | HAS_ARG | OPT_EXPERT, { &wanted_stream_spec[AVMEDIA_TYPE_VIDEO] }, "select desired video stream", "stream_specifier" },
    { "sst", OPT_STRING | HAS_ARG | OPT_EXPERT, { &wanted_stream_spec[AVMEDIA_TYPE_SUBTITLE] }, "select desired subtitle stream", "stream_specifier" },
    { "ss", HAS_ARG, { .func_arg = opt_seek }, "seek to a given position in seconds", "pos" },
    { "t", HAS_ARG, { .func_arg = opt_duration }, "play  \"duration\" seconds of audio/video", "duration" },
    { "bytes", OPT_INT | HAS_ARG, { &seek_by_bytes }, "seek by bytes 0=off 1=on -1=auto", "val" },
    { "nodisp", OPT_BOOL, { &display_disable }, "disable graphical display" },
    { "f", HAS_ARG, { .func_arg = opt_format }, "force format", "fmt" },
    { "pix_fmt", HAS_ARG | OPT_EXPERT | OPT_VIDEO, { .func_arg = opt_frame_pix_fmt }, "set pixel format", "format" },
    { "stats", OPT_BOOL | OPT_EXPERT, { &show_status }, "show status", "" },
    { "fast", OPT_BOOL | OPT_EXPERT, { &fast }, "non spec compliant optimizations", "" },
    { "genpts", OPT_BOOL | OPT_EXPERT, { &genpts }, "generate pts", "" },
    { "drp", OPT_INT | HAS_ARG | OPT_EXPERT, { &decoder_reorder_pts }, "let decoder reorder pts 0=off 1=on -1=auto", ""},
    { "lowres", OPT_INT | HAS_ARG | OPT_EXPERT, { &lowres }, "", "" },
    { "sync", HAS_ARG | OPT_EXPERT, { .func_arg = opt_sync }, "set audio-video sync. type (type=audio/video/ext)", "type" },
    { "autoexit", OPT_BOOL | OPT_EXPERT, { &autoexit }, "exit at the end", "" },
    { "exitonkeydown", OPT_BOOL | OPT_EXPERT, { &exit_on_keydown }, "exit on key down", "" },
    { "exitonmousedown", OPT_BOOL | OPT_EXPERT, { &exit_on_mousedown }, "exit on mouse down", "" },
    { "loop", OPT_INT | HAS_ARG | OPT_EXPERT, { &loop }, "set number of times the playback shall be looped", "loop count" },
    { "framedrop", OPT_BOOL | OPT_EXPERT, { &framedrop }, "drop frames when cpu is too slow", "" },
    { "infbuf", OPT_BOOL | OPT_EXPERT, { &infinite_buffer }, "don't limit the input buffer size (useful with realtime streams)", "" },
    { "window_title", OPT_STRING | HAS_ARG, { &window_title }, "set window title", "window title" },
    { "rdftspeed", OPT_INT | HAS_ARG| OPT_AUDIO | OPT_EXPERT, { &rdftspeed }, "rdft speed", "msecs" },
    { "showmode", HAS_ARG, { .func_arg = opt_show_mode}, "select show mode (0 = video, 1 = waves, 2 = RDFT)", "mode" },
    { "default", HAS_ARG | OPT_AUDIO | OPT_VIDEO | OPT_EXPERT, { .func_arg = opt_default }, "generic catch all option", "" },
    { "i", OPT_BOOL, { &dummy}, "read specified file", "input_file"},
    { "codec", HAS_ARG, { .func_arg = opt_codec}, "force decoder", "decoder_name" },
    { "acodec", HAS_ARG | OPT_STRING | OPT_EXPERT, {    &audio_codec_name }, "force audio decoder",    "decoder_name" },
    { "scodec", HAS_ARG | OPT_STRING | OPT_EXPERT, { &subtitle_codec_name }, "force subtitle decoder", "decoder_name" },
    { "vcodec", HAS_ARG | OPT_STRING | OPT_EXPERT, {    &video_codec_name }, "force video decoder",    "decoder_name" },
    { "autorotate", OPT_BOOL, { &autorotate }, "automatically rotate video", "" },
    { NULL, },
};

#endif // OPT_H_INCLUDED
