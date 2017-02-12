#include "decode.h"

void decoder_init(Decoder *d, AVCodecContext *avctx, PacketQueue *queue, SDL_cond *empty_queue_cond)
{
    memset(d, 0, sizeof(Decoder));
    d->avctx = avctx;
    d->queue = queue;
    d->empty_queue_cond = empty_queue_cond;
    d->start_pts = AV_NOPTS_VALUE;
}

int decoder_decode_frame(Decoder *d, AVFrame *frame, AVSubtitle *sub)
{
    int got_frame = 0;

    do
    {
        int ret = -1;

        if (d->queue->abort_request)
            return -1;

        if (!d->packet_pending || d->queue->serial != d->pkt_serial)//解码完一帧或者包队列又添新包,则继续取包
        {
            AVPacket pkt;
            do
            {
                if (d->queue->nb_packets == 0)
                    SDL_CondSignal(d->empty_queue_cond);
                if (packet_queue_get(d->queue, &pkt, 1, &d->pkt_serial) < 0)
                    return -1;
                if (pkt.data == flush_pkt.data)
                {
                    avcodec_flush_buffers(d->avctx);
                    d->finished = 0;
                    d->next_pts = d->start_pts;
                    d->next_pts_tb = d->start_pts_tb;
                }
            }
            while (pkt.data == flush_pkt.data || d->queue->serial != d->pkt_serial);
            av_packet_unref(&d->pkt);
            d->pkt_temp = d->pkt = pkt;
            d->packet_pending = 1;
        }

        switch (d->avctx->codec_type)
        {
        case AVMEDIA_TYPE_VIDEO:
            ret = avcodec_decode_video2(d->avctx, frame, &got_frame, &d->pkt_temp);
            if (got_frame)
            {
                if (decoder_reorder_pts == -1)
                {
                    frame->pts = av_frame_get_best_effort_timestamp(frame);
                }
                else if (decoder_reorder_pts)
                {
                    frame->pts = frame->pkt_pts;
                }
                else
                {
                    frame->pts = frame->pkt_dts;
                }
            }
            break;
        case AVMEDIA_TYPE_AUDIO:
            ret = avcodec_decode_audio4(d->avctx, frame, &got_frame, &d->pkt_temp);
            if (got_frame)
            {
                AVRational tb = (AVRational)
                {
                    1, frame->sample_rate
                };
                if (frame->pts != AV_NOPTS_VALUE)
                    frame->pts = av_rescale_q(frame->pts, d->avctx->time_base, tb);
                else if (frame->pkt_pts != AV_NOPTS_VALUE)
                    frame->pts = av_rescale_q(frame->pkt_pts, av_codec_get_pkt_timebase(d->avctx), tb);
                else if (d->next_pts != AV_NOPTS_VALUE)
                    frame->pts = av_rescale_q(d->next_pts, d->next_pts_tb, tb);
                if (frame->pts != AV_NOPTS_VALUE)
                {
                    d->next_pts = frame->pts + frame->nb_samples;
                    d->next_pts_tb = tb;
                }
            }
            break;
        case AVMEDIA_TYPE_SUBTITLE:
            ret = avcodec_decode_subtitle2(d->avctx, sub, &got_frame, &d->pkt_temp);
            break;
        }

        if (ret < 0)
        {
            d->packet_pending = 0;
        }
        else
        {
            d->pkt_temp.dts =
                d->pkt_temp.pts = AV_NOPTS_VALUE;
            if (d->pkt_temp.data)
            {
                if (d->avctx->codec_type != AVMEDIA_TYPE_AUDIO)
                    ret = d->pkt_temp.size;
                d->pkt_temp.data += ret;
                d->pkt_temp.size -= ret;
                if (d->pkt_temp.size <= 0)
                    d->packet_pending = 0;
            }
            else
            {
                if (!got_frame)
                {
                    d->packet_pending = 0;
                    d->finished = d->pkt_serial;
                }
            }
        }
    }
    while (!got_frame && !d->finished);

    return got_frame;
}

int subtitle_thread(void *arg)
{
    VideoState *is = arg;
    Frame *sp;
    int got_subtitle;
    double pts;
    int i;

    for (;;)
    {
        if (!(sp = frame_queue_peek_writable(&is->subpq)))
            return 0;

        if ((got_subtitle = decoder_decode_frame(&is->subdec, NULL, &sp->sub)) < 0)
            break;

        pts = 0;

        if (got_subtitle && sp->sub.format == 0)
        {
            if (sp->sub.pts != AV_NOPTS_VALUE)
                pts = sp->sub.pts / (double)AV_TIME_BASE;
            sp->pts = pts;
            sp->serial = is->subdec.pkt_serial;
            if (!(sp->subrects = av_mallocz_array(sp->sub.num_rects, sizeof(AVSubtitleRect*))))
            {
                av_log(NULL, AV_LOG_FATAL, "Cannot allocate subrects\n");
                exit(1);
            }

            for (i = 0; i < sp->sub.num_rects; i++)
            {
                int in_w = sp->sub.rects[i]->w;
                int in_h = sp->sub.rects[i]->h;
                int subw = is->subdec.avctx->width  ? is->subdec.avctx->width  : is->viddec_width;
                int subh = is->subdec.avctx->height ? is->subdec.avctx->height : is->viddec_height;
                int out_w = is->viddec_width  ? in_w * is->viddec_width  / subw : in_w;
                int out_h = is->viddec_height ? in_h * is->viddec_height / subh : in_h;

                if (!(sp->subrects[i] = av_mallocz(sizeof(AVSubtitleRect))) ||
                        av_image_alloc(sp->subrects[i]->data, sp->subrects[i]->linesize, out_w, out_h, AV_PIX_FMT_YUVA420P, 16) < 0)
                {
                    av_log(NULL, AV_LOG_FATAL, "Cannot allocate subtitle data\n");
                    exit(1);
                }

                is->sub_convert_ctx = sws_getCachedContext(is->sub_convert_ctx,
                                      in_w, in_h, AV_PIX_FMT_PAL8, out_w, out_h,
                                      AV_PIX_FMT_YUVA420P, sws_flags, NULL, NULL, NULL);
                if (!is->sub_convert_ctx)
                {
                    av_log(NULL, AV_LOG_FATAL, "Cannot initialize the sub conversion context\n");
                    exit(1);
                }
                sws_scale(is->sub_convert_ctx,
                          (void*)sp->sub.rects[i]->data, sp->sub.rects[i]->linesize,
                          0, in_h, sp->subrects[i]->data, sp->subrects[i]->linesize);

                sp->subrects[i]->w = out_w;
                sp->subrects[i]->h = out_h;
                sp->subrects[i]->x = sp->sub.rects[i]->x * out_w / in_w;
                sp->subrects[i]->y = sp->sub.rects[i]->y * out_h / in_h;
            }

            /* now we can update the picture count */
            frame_queue_push(&is->subpq);
        }
        else if (got_subtitle)
        {
            avsubtitle_free(&sp->sub);
        }
    }
    return 0;
}

int audio_thread(void *arg)
{
    VideoState *is = arg;
    AVFrame *frame = av_frame_alloc();
    Frame *af;

    int got_frame = 0;
    AVRational tb;
    int ret = 0;

    if (!frame)
        return AVERROR(ENOMEM);

    do
    {
        if ((got_frame = decoder_decode_frame(&is->auddec, frame, NULL)) < 0)
            goto the_end;

        if (got_frame)
        {
            tb = (AVRational)
            {
                1, frame->sample_rate
            };

            if (!(af = frame_queue_peek_writable(&is->sampq)))
                goto the_end;

            af->pts = (frame->pts == AV_NOPTS_VALUE) ? NAN : frame->pts * av_q2d(tb);
            af->pos = av_frame_get_pkt_pos(frame);
            af->serial = is->auddec.pkt_serial;
            af->duration = av_q2d((AVRational)
            {
                frame->nb_samples, frame->sample_rate
            });

            av_frame_move_ref(af->frame, frame);
            frame_queue_push(&is->sampq);

        }
    }
    while (ret >= 0 || ret == AVERROR(EAGAIN) || ret == AVERROR_EOF);
the_end:
    av_frame_free(&frame);
    return ret;
}

int get_video_frame(VideoState *is, AVFrame *frame)
{
    int got_picture;

    if ((got_picture = decoder_decode_frame(&is->viddec, frame, NULL)) < 0)
        return -1;

    if (got_picture)
    {
        double dpts = NAN;

        if (frame->pts != AV_NOPTS_VALUE)
            dpts = av_q2d(is->video_st->time_base) * frame->pts;

        frame->sample_aspect_ratio = av_guess_sample_aspect_ratio(is->ic, is->video_st, frame);

        is->viddec_width  = frame->width;
        is->viddec_height = frame->height;

        if (framedrop>0 || (framedrop && get_master_sync_type(is) != AV_SYNC_VIDEO_MASTER))//如果framedrop为1，则开启丢帧策略
        {
            if (frame->pts != AV_NOPTS_VALUE)
            {
                double diff = dpts - get_master_clock(is);
                if (!isnan(diff) && fabs(diff) < AV_NOSYNC_THRESHOLD &&
                        diff - is->frame_last_filter_delay < 0 &&
                        is->viddec.pkt_serial == is->vidclk.serial &&
                        is->videoq.nb_packets)
                {
                    is->frame_drops_early++;
                    av_frame_unref(frame);
                    got_picture = 0;
                }
            }
        }
    }

    return got_picture;
}

void duplicate_right_border_pixels(SDL_Overlay *bmp)
{
    int i, width, height;
    Uint8 *p, *maxp;
    for (i = 0; i < 3; i++)
    {
        width  = bmp->w;
        height = bmp->h;
        if (i > 0)
        {
            width  >>= 1;
            height >>= 1;
        }
        if (bmp->pitches[i] > width)
        {
            maxp = bmp->pixels[i] + bmp->pitches[i] * height - 1;
            for (p = bmp->pixels[i] + width - 1; p < maxp; p += bmp->pitches[i])
                *(p+1) = *p;
        }
    }
}


int queue_picture(VideoState *is, AVFrame *src_frame, double pts, double duration, int64_t pos, int serial)
{
    Frame *vp;

#if defined(DEBUG_SYNC) && 0
    printf("frame_type=%c pts=%0.3f\n",
           av_get_picture_type_char(src_frame->pict_type), pts);
#endif

    if (!(vp = frame_queue_peek_writable(&is->pictq)))
        return -1;

    vp->sar = src_frame->sample_aspect_ratio;

    /* alloc or resize hardware picture buffer */
    if (!vp->bmp || vp->reallocate || !vp->allocated ||
            vp->width  != src_frame->width ||
            vp->height != src_frame->height)
    {
        SDL_Event event;

        vp->allocated  = 0;
        vp->reallocate = 0;
        vp->width = src_frame->width;
        vp->height = src_frame->height;

        /* the allocation must be done in the main thread to avoid
           locking problems. */
        event.type = FF_ALLOC_EVENT;
        event.user.data1 = is;
        SDL_PushEvent(&event);

        /* wait until the picture is allocated */
        SDL_LockMutex(is->pictq.mutex);
        while (!vp->allocated && !is->videoq.abort_request)
        {
            SDL_CondWait(is->pictq.cond, is->pictq.mutex);
        }
        /* if the queue is aborted, we have to pop the pending ALLOC event or wait for the allocation to complete */
        if (is->videoq.abort_request && SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_EVENTMASK(FF_ALLOC_EVENT)) != 1)
        {
            while (!vp->allocated && !is->abort_request)
            {
                SDL_CondWait(is->pictq.cond, is->pictq.mutex);
            }
        }
        SDL_UnlockMutex(is->pictq.mutex);

        if (is->videoq.abort_request)
            return -1;
    }

    /* if the frame is not skipped, then display it */
    if (vp->bmp)
    {
        uint8_t *data[4];
        int linesize[4];

        /* get a pointer on the bitmap */
        SDL_LockYUVOverlay (vp->bmp);

        data[0] = vp->bmp->pixels[0];
        data[1] = vp->bmp->pixels[2];
        data[2] = vp->bmp->pixels[1];

        linesize[0] = vp->bmp->pitches[0];
        linesize[1] = vp->bmp->pitches[2];
        linesize[2] = vp->bmp->pitches[1];

        {
            AVDictionaryEntry *e = av_dict_get(sws_dict, "sws_flags", NULL, 0);
            if (e)
            {
                const AVClass *class = sws_get_class();
                const AVOption    *o = av_opt_find(&class, "sws_flags", NULL, 0,
                                                   AV_OPT_SEARCH_FAKE_OBJ);
                int ret = av_opt_eval_flags(&class, o, e->value, &sws_flags);
                if (ret < 0)
                    exit(1);
            }
        }

        is->img_convert_ctx = sws_getCachedContext(is->img_convert_ctx,
                              vp->width, vp->height, src_frame->format, vp->width, vp->height,
                              AV_PIX_FMT_YUV420P, sws_flags, NULL, NULL, NULL);
        if (!is->img_convert_ctx)
        {
            av_log(NULL, AV_LOG_FATAL, "Cannot initialize the conversion context\n");
            exit(1);
        }
        sws_scale(is->img_convert_ctx, src_frame->data, src_frame->linesize,
                  0, vp->height, data, linesize);

        /* workaround SDL PITCH_WORKAROUND */
        duplicate_right_border_pixels(vp->bmp);
        /* update the bitmap content */
        SDL_UnlockYUVOverlay(vp->bmp);

        vp->pts = pts;
        vp->duration = duration;
        vp->pos = pos;
        vp->serial = serial;

        /* now we can update the picture count */
        frame_queue_push(&is->pictq);
    }
    return 0;
}

int video_thread(void *arg)
{
    VideoState *is = arg;
    AVFrame *frame = av_frame_alloc();
    double pts;
    double duration;
    int ret;
    AVRational tb = is->video_st->time_base;
    AVRational frame_rate = av_guess_frame_rate(is->ic, is->video_st, NULL);

    if (!frame)
    {
        return AVERROR(ENOMEM);
    }

    for (;;)
    {
        ret = get_video_frame(is, frame);
        if (ret < 0)
            goto the_end;
        if (!ret)
            continue;

        duration = (frame_rate.num && frame_rate.den ? av_q2d((AVRational)
        {
            frame_rate.den, frame_rate.num
        }) : 0);
        pts = (frame->pts == AV_NOPTS_VALUE) ? NAN : frame->pts * av_q2d(tb);
        ret = queue_picture(is, frame, pts, duration, av_frame_get_pkt_pos(frame), is->viddec.pkt_serial);
        av_frame_unref(frame);

        if (ret < 0)
            goto the_end;
    }
the_end:
    av_frame_free(&frame);
    return 0;
}

int decoder_start(Decoder *d, int (*fn)(void *), void *arg)
{
    packet_queue_start(d->queue);
    d->decoder_tid = SDL_CreateThread(fn, arg);
    if (!d->decoder_tid) {
        av_log(NULL, AV_LOG_ERROR, "SDL_CreateThread(): %s\n", SDL_GetError());
        return AVERROR(ENOMEM);
    }
    return 0;
}

