/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 */

// Based on tutorial08.c
// A pedagogical video player that really works!
//
// Code based on FFplay, Copyright (c) 2003 Fabrice Bellard, 
// and a tutorial by Martin Bohme (boehme@inb.uni-luebeckREMOVETHIS.de)


#include <cassert>
#include <algorithm>
#include <mutex>
#include <thread>
#include <vector>
#include <windows.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
#include <libavutil/time.h>
#include <libavutil/opt.h>
}

#include "lgviddecoder.h"

// BBi
#include "lgvs_lgvid_subs.h"


#ifdef _DEBUG
#include <assert.h>
#define Warning(_x) FFmpeg::mprintf _x
#define Assert_(_x) assert(_x)
#define AssertMsg(_x, _msg) assert(_x)
#define AssertMsg1(_x, _msg, _a1) assert(_x)
#else
#define Warning(_x)
#define Assert_(_x)
#define AssertMsg(_x, _msg)
#define AssertMsg1(_x, _msg, _a1)
#endif


typedef unsigned char uint8;
typedef unsigned int uint32;
typedef unsigned long ulong;
#ifndef QWORD
typedef unsigned long long QWORD;
#endif


#define AUDIO_BUFFER_SIZE (16 * 1024)
#define MIN_AUDIOQ_SIZE (20 * 16 * 1024)
#define MIN_FRAMES (5)
#define AV_SYNC_THRESHOLD (0.01)
#define AV_NOSYNC_THRESHOLD (10.0)
#define SAMPLE_CORRECTION_PERCENT_MAX (10)
#define AUDIO_DIFF_AVG_NB (20)
#define FF_ALLOC_EVENT (SDL_USEREVENT)
#define FF_REFRESH_EVENT (SDL_USEREVENT + 1)
#define FF_QUIT_EVENT (SDL_USEREVENT + 2)
#define VIDEO_PICTURE_QUEUE_SIZE (2)
#define DEFAULT_AV_SYNC_TYPE AV_SYNC_EXTERNAL_MASTER

static int sws_flags = SWS_BICUBIC;


#ifdef _DEBUG
// uncomment to simulate movie playback on slower CPU
//#define SIMULATE_SLOW_CPU

// simulated extra time (ms) for an audio packet to decode
#define AUDIO_DECODE_STALL (1000 / 60)
// simluated extra time (ms) for a video frame to decode
#define VIDEO_DECODE_STALL (1000 / 25)
#endif

///////////////////////////////////////////////////////////////////////////////

struct VideoState;

class cLGVideoDecoder : public ILGVideoDecoder {
public:
    ILGVideoDecoderHost *m_pHostIface;

    VideoState *is;

public:
    cLGVideoDecoder(ILGVideoDecoderHost *pHostIface);
    virtual ~cLGVideoDecoder();

    // ILGVideoDecoder interface implementation
    STDMETHOD_(void, Destroy)();
    STDMETHOD_(BOOL, Start)();
    STDMETHOD_(BOOL, IsFinished)();
    STDMETHOD_(BOOL, IsVideoFrameAvailable)();
    STDMETHOD_(void, RequestVideoFrame)();
    STDMETHOD_(void, RequestAudio)(unsigned int len);

    void Stop();
    BOOL Init(const char* filename);
};


///////////////////////////////////////////////////////////////////////////////
//
// FFmpeg dynamic lib API
//

namespace FFmpeg {
    cLGVideoDecoder *pOuter = NULL;

    static void mprintf(const char *fmt, ...)
    {
        char buf[1536];

        if (pOuter) {
            va_list ap;
            va_start(ap, fmt);
            vsprintf(buf, fmt, ap);
            va_end(ap);

            pOuter->m_pHostIface->LogPrint(buf);
        }
    }

    void Shutdown()
    {
        pOuter = NULL;
    }

    BOOL Init(cLGVideoDecoder *pOuter_)
    {
        pOuter = pOuter_;
        return TRUE;
    }

    //
    // I/O interface
    //

    struct IOContext {
        void *pStream;
        ulong size;

        IOContext(void *str) { pStream = str; size = pOuter->m_pHostIface->FileSize(str); }
    };

    static int Read(void *opaque, uint8_t *buf, int buf_size)
    {
        IOContext *io = (IOContext*) opaque;
        return pOuter->m_pHostIface->FileRead(io->pStream, buf, buf_size);
    }

    static int64_t Seek(void *opaque, int64_t offset, int whence)
    {
        IOContext *io = (IOContext*) opaque;
        return (whence == AVSEEK_SIZE) ? (QWORD) io->size : pOuter->m_pHostIface->FileSeek(io->pStream, static_cast<long>(offset), whence);
    }


    int OpenFile(AVFormatContext **ic_ptr, const char *filename)
    {
        void *pStream = pOuter->m_pHostIface->FileOpen(filename);
        if (!pStream) {
            mprintf("failed to open file '%s'", filename);
            return -1;
        }

        int ret = 0;

        IOContext *ctxt = new IOContext(pStream);

        AVIOContext *pb = avio_alloc_context(NULL, 0, 0, ctxt, Read, NULL/*Write*/, Seek);

        (*ic_ptr)->pb = pb;
        ret = avformat_open_input(ic_ptr, filename, nullptr, nullptr);

        if (ret) {
            pOuter->m_pHostIface->FileClose(pStream);
            delete ctxt;
            av_free(pb);
        }

        return ret;
    }

    void CloseFile(AVFormatContext *s)
    {
        Assert_(s != NULL);

        AVIOContext *pb = s->pb;

        if (pb) {
            if (pb->opaque) {
                IOContext *io = (IOContext*) pb->opaque;

                if (io->pStream)
                    pOuter->m_pHostIface->FileClose(io->pStream);

                delete io;
                pb->opaque = NULL;
            }

            av_free(pb);
        }

        avformat_close_input(&s);
    }
};


#pragma pack(8)


///////////////////////////////////////////////////////////////////////////////
// Thread Utilities

class cThreadSyncObject {
public:
    ~cThreadSyncObject() { if (m_hSyncObject) CloseHandle(m_hSyncObject); }
    BOOL operator!() const { return !m_hSyncObject; }
    operator HANDLE () { return m_hSyncObject; }
    BOOL Wait(DWORD dwTimeout = INFINITE) { return WaitForSingleObject(m_hSyncObject, dwTimeout) == WAIT_OBJECT_0; }
protected:
    cThreadSyncObject() : m_hSyncObject(NULL) {}
    HANDLE m_hSyncObject;
};

class cThreadSemaphore : public cThreadSyncObject {
public:
    cThreadSemaphore(long initialValue, long maxValue) { m_hSyncObject = CreateSemaphore(NULL, initialValue, maxValue, NULL); }
    BOOL Release(long releaseCount = 1, long * pPreviousCount = NULL) { return ReleaseSemaphore(m_hSyncObject, releaseCount, pPreviousCount); }
};

class WorkerThread {
public:
    WorkerThread() :
        thread_()
    {
    }

    virtual ~WorkerThread()
    {
        if (thread_ != nullptr)
            wait_for_close();
    }

    bool create()
    {
        if (thread_ != nullptr)
            return false;

        try {
            thread_ = new std::thread(
                WorkerThread::thread_proc_wrapper,
                this);
        } catch (const std::system_error&) {
        }

        return thread_ != nullptr;
    }

    void wait_for_close()
    {
        if (thread_ == nullptr)
            return;

        thread_->join();
        thread_ = nullptr;
    }

protected:
    virtual void thread_proc() = 0;

private:
    static void thread_proc_wrapper(
        void* pv)
    {
        auto pThread = static_cast<WorkerThread*>(pv);
        pThread->thread_proc();
    }

    std::thread* thread_;
};

///////////////////////////////////////////////////////////////////////////////
// SDL_Cond

struct SDL_cond {
    std::mutex lock;

    int waiting;
    int signals;
    cThreadSemaphore wait_sem;
    cThreadSemaphore wait_done;

    SDL_cond() : wait_sem(0, 32 * 1024), wait_done(0, 32 * 1024)
    {
        waiting = signals = 0;
    }
};


int SDL_CondSignal(SDL_cond &cond)
{
    cond.lock.lock();

    if (cond.waiting > cond.signals) {
        ++cond.signals;
        cond.wait_sem.Release();
        cond.lock.unlock();
        cond.wait_done.Wait();
    } else {
        cond.lock.unlock();
    }

    return 0;
}

int SDL_CondWaitTimeout(
    SDL_cond& cond,
    std::mutex& mutex,
    uint32 ms)
{
    int retval;

    cond.lock.lock();
    ++cond.waiting;
    cond.lock.unlock();

    mutex.unlock();

    retval = !cond.wait_sem.Wait(ms);

    cond.lock.lock();

    if (cond.signals > 0) {
        if (retval > 0)
            cond.wait_sem.Wait();
        cond.wait_done.Release();
        --cond.signals;
    }

    --cond.waiting;
    cond.lock.unlock();

    mutex.lock();

    return retval;
}

inline int SDL_CondWait(
    SDL_cond& cond,
    std::mutex& mutex)
{
    return SDL_CondWaitTimeout(cond, mutex, ~0U);
}


///////////////////////////////////////////////////////////////////////////////
//
// PacketQueue
//

struct PacketQueue {
    PacketQueue() : first_pkt(0), last_pkt(0), nb_packets(0), size(0), quit(0), finished(0) {};
    ~PacketQueue()
    {
        Flush();
    }


    AVPacketList *first_pkt, *last_pkt;
    int nb_packets;
    int size;
    std::mutex mutex;
    SDL_cond cond;

    int Put(AVPacket *pkt)
    {
        Assert_(!finished && !quit);

        AVPacketList *pkt1;
        if (av_dup_packet(pkt) < 0) {
            return -1;
        }

        pkt1 = (AVPacketList*) av_malloc(sizeof(AVPacketList));
        if (!pkt1)
            return -1;
        pkt1->pkt = *pkt;
        pkt1->next = NULL;

        mutex.lock();

        if (!last_pkt)
            first_pkt = pkt1;
        else
            last_pkt->next = pkt1;
        last_pkt = pkt1;
        nb_packets++;
        size += pkt1->pkt.size + sizeof(*pkt1);

        SDL_CondSignal(cond);

        mutex.unlock();
        return 0;
    }

    int Get(AVPacket *pkt, int block)
    {
        AVPacketList *pkt1;
        int ret;

        mutex.lock();

        for (;;) {
            if (quit) {
                ret = -1;
                break;
            }

            pkt1 = first_pkt;
            if (pkt1) {
                first_pkt = pkt1->next;
                if (!first_pkt)
                    last_pkt = NULL;
                nb_packets--;
                size -= pkt1->pkt.size + sizeof(*pkt1);
                *pkt = pkt1->pkt;
                av_free(pkt1);
                ret = 1;
                break;
            } else if (!block) {
                ret = 0;
                break;
            } else {
                // If finished and queue is empty, let receiver know
                if (finished) {
                    ret = -1;
                    break;
                }

                SDL_CondWait(cond, mutex);
            }
        }
        mutex.unlock();
        return ret;
    }

    void Flush()
    {
        AVPacketList *pkt, *pkt1;

        mutex.lock();
        for (pkt = first_pkt; pkt != NULL; pkt = pkt1) {
            pkt1 = pkt->next;
            av_free_packet(&pkt->pkt);
            av_freep(&pkt);
        }
        last_pkt = NULL;
        first_pkt = NULL;
        nb_packets = 0;
        size = 0;
        mutex.unlock();
    }

    void Quit(int ret = 1)
    {
        mutex.lock();
        quit = ret;

        SDL_CondSignal(cond);

        mutex.unlock();
    }

    void Finished(int ret = 1)
    {
        mutex.lock();
        finished = ret;

        SDL_CondSignal(cond);

        mutex.unlock();
    }

    void Depleted()
    {
        mutex.lock();

        finished = 1;
        if (!first_pkt)
            SDL_CondSignal(cond);

        mutex.unlock();
    }
private:
    int quit;
    int finished;
};


///////////////////////////////////////////////////////////////////////////////
//
// VideoState
//

struct VideoState {
    struct VideoPicture {
        void *bmp;
        double pts;				///<presentation time stamp for this picture
        double target_clock;	///<av_gettime() time at which this should be displayed ideally
        int64_t pos;			///<byte position in file
    };

    enum AV_SYNC {
        AV_SYNC_AUDIO_MASTER,
        AV_SYNC_VIDEO_MASTER,
        AV_SYNC_EXTERNAL_MASTER,
    };

    cLGVideoDecoder *pOuter;

    AVFormatContext *pFormatCtx;
    int             videoStream, audioStream;

    AV_SYNC         av_sync_type;
    double          external_clock; /* external clock base */
    int64_t         external_clock_time;

    double          audio_clock;
    AVStream        *audio_st;
    PacketQueue     audioq;
    AVFrame* audio_frame;
    std::vector<uint8_t> avr_buffer;
    uint8_t         *audio_buf;
    unsigned int    audio_buf_size;
    unsigned int    audio_buf_index;
    AVPacket        audio_pkt;
    AVPacket        audio_pkt2;
    AVSampleFormat  audio_src_fmt;
    SwrContext* avr_context;
    double          audio_diff_cum; /* used for AV difference average computation */
    double          audio_diff_avg_coef;
    double          audio_diff_threshold;
    int             audio_diff_avg_count;
    double          frame_timer;
    double          frame_last_pts;
    double          frame_last_delay;
    double          video_clock; ///<pts of last decoded frame / predicted pts of next decoded frame
    double          video_current_pts; ///<current displayed pts (different from video_clock if frame fifos are used)
    double          video_current_pts_drift; ///<video_current_pts - time (av_gettime) at which we updated video_current_pts - used to have running video pts
    int64_t         video_current_pos; ///<current displayed file pos
    int64_t         video_current_pts_time;  ///<time (av_gettime) at which we updated video_current_pts - used to have running video pts
    AVStream        *video_st;
    PacketQueue     videoq;

    PixelFormat     pict_pix_fmt;
    VideoPicture    pictq[VIDEO_PICTURE_QUEUE_SIZE];
    int             pictq_size, pictq_rindex, pictq_windex;
    std::mutex pictq_mutex;
    SDL_cond        pictq_cond;
    class DecodeThread     *parse_tid;
    class VideoThread      *video_tid;
    int             quit;
    SwsContext      *img_convert_ctx;
    HANDLE			timer;

    int				decode_finished;
    int				video_finished;
    int				audio_finished;

    float           skip_frames;
    float           skip_frames_index;
    int             refresh;

    // BBi
    lgvs::LgVidSubs subs;

    VideoState(cLGVideoDecoder *pOuter_) :
        pOuter(pOuter_),
        pFormatCtx(),
        videoStream(-1),
        audioStream(-1),
        av_sync_type(DEFAULT_AV_SYNC_TYPE),
        external_clock(),
        external_clock_time(),
        audio_clock(),
        audio_st(),
        audio_buf_size(),
        audio_buf_index(),
        audio_src_fmt(AV_SAMPLE_FMT_NONE),
        avr_context(),
        audio_diff_cum(),
        audio_diff_avg_coef(),
        audio_diff_threshold(),
        audio_diff_avg_count(),
        frame_timer(),
        frame_last_pts(),
        frame_last_delay(),
        video_clock(),
        video_current_pts(),
        video_current_pts_drift(),
        video_current_pos(),
        video_current_pts_time(),
        video_st(),
        pictq_size(),
        pictq_rindex(),
        pictq_windex(),
        parse_tid(),
        video_tid(),
        quit(),
        img_convert_ctx(),
        decode_finished(),
        video_finished(),
        audio_finished(),
        skip_frames(),
        skip_frames_index(),
        refresh()

        // BBi
        ,
        subs()
        // BBi
    {
        audio_frame = av_frame_alloc();

        av_init_packet(&audio_pkt);
        memset(pictq, 0, sizeof(pictq));
    }

    ~VideoState()
    {
        Close();
    }

    void schedule_refresh(int delay)
    {
    }

    double get_audio_clock() const
    {
        double pts;
        int hw_buf_size, bytes_per_sec;
        pts = audio_clock;
        hw_buf_size = audio_buf_size - audio_buf_index;
        bytes_per_sec = 0;
        if (audio_st) {
            bytes_per_sec = audio_st->codec->sample_rate *
                2 * audio_st->codec->channels;
        }
        if (bytes_per_sec)
            pts -= (double) hw_buf_size / bytes_per_sec;
        return pts;
    }

    void DecodeFinished()
    {
        videoq.Finished();
        audioq.Finished();
        decode_finished = 1;
    }

    void VideoFinished()
    {
        video_finished = 1;
    }

    void AudioFinished()
    {
        audio_finished = 1;
    }

    double get_video_clock() const
    {
        return video_current_pts_drift + av_gettime() / 1000000.0;
    }

    double get_external_clock() const
    {
        const int64_t ti = av_gettime();
        return external_clock + ((ti - external_clock_time) * 1e-6);
    }

    double get_master_clock() const
    {
        double val;

        if (av_sync_type == AV_SYNC_VIDEO_MASTER) {
            if (video_st)
                val = get_video_clock();
            else
                val = get_audio_clock();
        } else if (av_sync_type == AV_SYNC_AUDIO_MASTER) {
            if (audio_st)
                val = get_audio_clock();
            else
                val = get_video_clock();
        } else {
            val = get_external_clock();
        }

        return val;
    }

    BOOL Open(const char *filename, int target_w, int target_h, PixelFormat dst_pix_fmt)
    {
        Close();

        pFormatCtx = avformat_alloc_context();

        // Open video file
        if (FFmpeg::OpenFile(&pFormatCtx, filename) != 0)
            return FALSE; // Couldn't open file

        // Retrieve stream information
        if (avformat_find_stream_info(pFormatCtx, nullptr) < 0)
            return FALSE; // Couldn't find stream information

        if (pFormatCtx->pb)
            pFormatCtx->pb->eof_reached = 0;

#if defined(_DEBUG) || defined(DEBUG)
        // Dump information about file onto standard error
        av_dump_format(pFormatCtx, 0, filename, 0);
#endif

        // Find the first video stream

        int st_index[AVMEDIA_TYPE_NB];
        memset(st_index, -1, sizeof(st_index));

        for (unsigned int i = 0; i < pFormatCtx->nb_streams; i++)
            pFormatCtx->streams[i]->discard = AVDISCARD_ALL;

        st_index[AVMEDIA_TYPE_VIDEO] = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
        st_index[AVMEDIA_TYPE_AUDIO] = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_AUDIO, -1, st_index[AVMEDIA_TYPE_VIDEO], NULL, 0);

        // Open streams

        if (st_index[AVMEDIA_TYPE_AUDIO] >= 0) {
            stream_component_open(st_index[AVMEDIA_TYPE_AUDIO]);
        }
        if (st_index[AVMEDIA_TYPE_VIDEO] >= 0) {
            stream_component_open(st_index[AVMEDIA_TYPE_VIDEO]);
        }

        if (videoStream < 0) {
            AssertMsg1(FALSE, "%s: could not open codecs\n", filename);
            return FALSE;
        }

        // Init video scaler

        pict_pix_fmt = dst_pix_fmt;

        // Allocate frame buffer(s)
        for (int i = 0; i < VIDEO_PICTURE_QUEUE_SIZE; ++i)
            pictq[i].bmp = pOuter->m_pHostIface->CreateImageBuffer();

        // BBi
        subs.initialize(pOuter->m_pHostIface, filename);

        return TRUE;
    }

    void Stop();

    void Close()
    {
        Stop();

        swr_free(&avr_context);

        videoStream = -1;
        audioStream = -1;

        videoq.Flush();
        audioq.Flush();

        // Close the codec
        if (video_st)
            avcodec_close(video_st->codec);
        video_st = NULL;

        if (audio_st)
            avcodec_close(audio_st->codec);
        audio_st = NULL;

        // Close the video file
        if (pFormatCtx)
            FFmpeg::CloseFile(pFormatCtx);
        pFormatCtx = NULL;

        if (img_convert_ctx)
            sws_freeContext(img_convert_ctx);
        img_convert_ctx = NULL;

        for (int i = 0; i < VIDEO_PICTURE_QUEUE_SIZE; ++i)
            pictq[i].bmp = NULL;
    }

    BOOL Play();

    void video_refresh_timer()
    {
        VideoPicture *vp;

        if (video_st) {
        retry:
            if (pictq_size == 0) {
                schedule_refresh(1);
            } else {
                double time = av_gettime() / 1000000.0;
                double next_target;

                /* dequeue the picture */
                vp = &pictq[pictq_rindex];

                if (time < vp->target_clock)
                    return;

                /* update current video pts */
                video_current_pts = vp->pts;
                video_current_pts_drift = video_current_pts - time;
                video_current_pos = vp->pos;

                if (pictq_size > 1) {
                    VideoPicture *nextvp = &pictq[(pictq_rindex + 1) % VIDEO_PICTURE_QUEUE_SIZE];
                    Assert_(nextvp->target_clock >= vp->target_clock);
                    next_target = nextvp->target_clock;
                } else {
                    next_target = vp->target_clock + video_clock - vp->pts; //FIXME pass durations cleanly
                }

#define FRAME_SKIP_FACTOR 0.05F
                /**/const BOOL framedrop = TRUE;

                if (framedrop && time > next_target) {
                    skip_frames *= 1.0F + FRAME_SKIP_FACTOR;
                    if (pictq_size > 1 && time > next_target + 0.5) {
                        /* update queue size and signal for next picture */
                        if (++pictq_rindex == VIDEO_PICTURE_QUEUE_SIZE)
                            pictq_rindex = 0;

#ifdef _DEBUG
                        Warning(("ffmpeg: dropped display frame dt:%g q:%d sf:%g\n", next_target - time, pictq_size, skip_frames));
#endif

                        pictq_mutex.lock();
                        pictq_size--;
                        SDL_CondSignal(pictq_cond);
                        pictq_mutex.unlock();
                        goto retry;
                    }
                }

                // show the picture!

                // BBi
                if (!subs.has_subtitles()) {
                    // BBi

                    pOuter->m_pHostIface->BeginVideoFrame(pictq[pictq_rindex].bmp);

                    // update queue for next picture!
                    if (++pictq_rindex == VIDEO_PICTURE_QUEUE_SIZE) {
                        pictq_rindex = 0;
                    }

                    pictq_mutex.lock();
                    pictq_size--;
                    SDL_CondSignal(pictq_cond);
                    pictq_mutex.unlock();

                    pOuter->m_pHostIface->EndVideoFrame();

                // BBi
                } else
                    subs.refresh_video = true;
                // BBi
            }
        } else
            schedule_refresh(100);
    }

    double compute_target_time(double frame_current_pts)
    {
        double delay, sync_threshold, diff;

        /* compute nominal delay */
        delay = frame_current_pts - frame_last_pts;
        if (delay <= 0 || delay >= 10.0) {
            /* if incorrect delay, use previous one */
            delay = frame_last_delay;
        } else {
            frame_last_delay = delay;
        }
        frame_last_pts = frame_current_pts;

        /* update delay to follow master synchronisation source */
        if (((av_sync_type == AV_SYNC_AUDIO_MASTER && audio_st) ||
            av_sync_type == AV_SYNC_EXTERNAL_MASTER)) {
            /* if video is slave, we try to correct big delays by
            duplicating or deleting a frame */
            diff = get_video_clock() - get_master_clock();

            /* skip or repeat frame. We take into account the
            delay to compute the threshold. I still don't know
            if it is the best guess */
            sync_threshold = FFMAX(AV_SYNC_THRESHOLD, delay);
            if (fabs(diff) < AV_NOSYNC_THRESHOLD) {
                if (diff <= -sync_threshold)
                    delay = 0;
                else if (diff >= sync_threshold)
                    delay = 2 * delay;
            }
        }
        frame_timer += delay;

        return frame_timer;
    }

    void audio_request(unsigned int len)
    {
        int len1, audio_size = 0;
        double pts;

        if (!len) {
            AudioFinished();
            return;
        }

        while (len > 0) {
            if (audio_buf_index >= audio_buf_size) {
                /* We have already sent all our data; get more */
                audio_size = audio_decode_frame(&pts);
                if (audio_size < 0) {
                    /* If error, output silence */
                    audio_buf_size = 1024;
                    memset(audio_buf, 0, audio_buf_size);
                } else {
                    audio_size = synchronize_audio((int16_t *) audio_buf,
                        audio_size, pts);
                    audio_buf_size = audio_size;
                }
                audio_buf_index = 0;
            }

            len1 = audio_buf_size - audio_buf_index;
            if (len1 > (int) len)
                len1 = len;

            if (len1 > 0)
                audio_buf_index += pOuter->m_pHostIface->QueueAudioData(audio_buf + audio_buf_index, len1);

            len -= len1;
        }
    }

private:
    int stream_component_open(int stream_index);

    /* Add or subtract samples to get a better sync, return new
    audio buffer size */
    int synchronize_audio(short *samples, int samples_size, double pts)
    {
        int n;
        double ref_clock;

        n = 2 * audio_st->codec->channels;

        if (av_sync_type != AV_SYNC_AUDIO_MASTER) {
            double diff, avg_diff;
            int wanted_size, min_size, max_size;

            ref_clock = get_master_clock();
            diff = get_audio_clock() - ref_clock;
            if (diff < AV_NOSYNC_THRESHOLD) {
                // accumulate the diffs
                audio_diff_cum = diff + audio_diff_avg_coef
                    * audio_diff_cum;
                if (audio_diff_avg_count < AUDIO_DIFF_AVG_NB) {
                    audio_diff_avg_count++;
                } else {
                    avg_diff = audio_diff_cum * (1.0 - audio_diff_avg_coef);
                    if (fabs(avg_diff) >= audio_diff_threshold) {
                        wanted_size = samples_size + ((int) (diff * audio_st->codec->sample_rate) * n);
                        min_size = samples_size * ((100 - SAMPLE_CORRECTION_PERCENT_MAX) / 100);
                        max_size = samples_size * ((100 + SAMPLE_CORRECTION_PERCENT_MAX) / 100);
                        if (wanted_size < min_size) {
                            wanted_size = min_size;
                        } else if (wanted_size > max_size) {
                            wanted_size = max_size;
                        }
                        if (wanted_size < samples_size) {
                            /* remove samples */
                            samples_size = wanted_size;
                        } else if (wanted_size > samples_size) {
                            uint8_t *samples_end, *q;
                            int nb;
                            /* add samples by copying final sample*/
                            nb = (samples_size - wanted_size);
                            samples_end = (uint8_t *) samples + samples_size - n;
                            q = samples_end + n;
                            while (nb > 0) {
                                memcpy(q, samples_end, n);
                                q += n;
                                nb -= n;
                            }
                            samples_size = wanted_size;
                        }
                    }
                }
            } else {
                /* difference is TOO big; reset diff stuff */
                audio_diff_avg_count = 0;
                audio_diff_cum = 0;
            }
        }
        return samples_size;
    }

    int audio_decode_frame(double *pts_ptr)
    {
        int len1, data_size, n;
        AVPacket *pkt = &audio_pkt;
        AVPacket *pkt2 = &audio_pkt2;
        AVCodecContext *dec = audio_st->codec;
        double pts;
        int failed_frames = 0;

        for (;;) {
            while (pkt2->size > 0) {
                int got_frame = 0;

                len1 = avcodec_decode_audio4(
                    dec, audio_frame, &got_frame, pkt2);

#ifdef SIMULATE_SLOW_CPU
                Sleep(AUDIO_DECODE_STALL);
#endif

                if (len1 < 0) {
                    /* if error, skip frame */
                    pkt2->size = 0;
                    // try to avoid inifinite loop if something goes wrong
                    if (++failed_frames > 512)
                        return -1;
                    break;
                }

                pkt2->data += len1;
                pkt2->size -= len1;

                if (dec->sample_fmt != audio_src_fmt &&
                    avr_context == nullptr) {
                    avr_context = swr_alloc();

                    auto channel_layout =
                        av_get_default_channel_layout(audio_frame->channels);

                    auto sample_rate = audio_frame->sample_rate;

                    int av_result = 0;
                    bool is_succeed = true;

                    //
                    av_result = av_opt_set_int(
                        avr_context,
                        "in_channel_layout",
                        channel_layout,
                        0); // search flags

                    is_succeed &= (av_result == 0);

                    //
                    av_result = av_opt_set_int(
                        avr_context,
                        "out_channel_layout",
                        channel_layout,
                        0); // search flags

                    is_succeed &= (av_result == 0);

                    //
                    av_result = av_opt_set_int(
                        avr_context,
                        "in_sample_rate",
                        sample_rate,
                        0); // search flags

                    is_succeed &= (av_result == 0);

                    //
                    av_result = av_opt_set_int(
                        avr_context,
                        "out_sample_rate",
                        sample_rate,
                        0); // search flags

                    is_succeed &= (av_result == 0);

                    //
                    av_result = av_opt_set_int(
                        avr_context,
                        "in_sample_fmt",
                        dec->sample_fmt,
                        0); // search flags

                    is_succeed &= (av_result == 0);

                    //
                    av_result = av_opt_set_int(
                        avr_context,
                        "out_sample_fmt",
                        audio_src_fmt,
                        0); // search flags

                    if (is_succeed) {
                        av_result = swr_init(avr_context);
                        is_succeed = (av_result == 0);
                    }

                    if (!is_succeed) {
                        swr_free(&avr_context);
                    }
                }

                if (avr_context != nullptr) {
                    int sample_size = 2 * audio_frame->channels;
                    int sample_count = audio_frame->nb_samples;
                    data_size = sample_size * audio_frame->nb_samples;

                    if (static_cast<int>(avr_buffer.size()) < data_size)
                        avr_buffer.resize(data_size);

                    audio_buf = &avr_buffer[0];
                    uint8_t* lines[] = { audio_buf };

                    int av_result = swr_convert(
                        avr_context,
                        lines,
                        sample_count,
                        const_cast<const uint8_t**>(audio_frame->data),
                        sample_count);

                    if (av_result != sample_count)
                        data_size = 0;
                } else {
                    int byte_depth = av_get_bytes_per_sample(
                        dec->sample_fmt);
                    int sample_size = byte_depth * audio_frame->channels;
                    data_size = sample_size * audio_frame->nb_samples;
                    audio_buf = audio_frame->data[0];
                }

                pts = audio_clock;
                *pts_ptr = pts;
                n = 2 * dec->channels;
                audio_clock += (double) data_size / (double) (n * dec->sample_rate);

                /* We have data, return it and come back for more later */
                return data_size;
            }

            /* free the current packet */
            if (pkt->data)
                av_free_packet(pkt);

            if (quit) {
                return -1;
            }

            /* read next packet */
            if (audioq.Get(pkt, 1) < 0) {
                return -1;
            }

            pkt2->data = pkt->data;
            pkt2->size = pkt->size;

            /* if update, update the audio clock w/pts */
            if (pkt->pts != AV_NOPTS_VALUE) {
                audio_clock = av_q2d(audio_st->time_base)*pkt->pts;
            }
        }
    }
};


///////////////////////////////////////////////////////////////////////////////
//
// DecodeThread
//

class DecodeThread : public WorkerThread {
public:
    DecodeThread(
        VideoState* is) :
            is(is)
    {
    };

protected:
    virtual void thread_proc()
    {
        AVPacket pkt;
        AVPacket *packet = &pkt;

        av_init_packet(packet);

        int eof = 0;

        // main decode loop
        for ( ; ; ) {
            if (is->quit) {
                break;
            }

            // if the queue are full, no need to read more
            if ((is->audioq.size > MIN_AUDIOQ_SIZE || is->audioStream < 0)
                && (is->videoq.nb_packets > MIN_FRAMES || is->videoStream < 0))
            {
                Sleep(10);
                continue;
            }

            if (eof) {
                // wait for queues to run empty
                Sleep(10);
                if (is->audioq.size + is->videoq.size == 0) {
                    break;
                }
                continue;
            }

            if (av_read_frame(is->pFormatCtx, packet) < 0) {
                eof = 1;
                // signal queues that nothing more is coming
                is->videoq.Depleted();
                is->audioq.Depleted();
                continue;
            }
            // Is this a packet from the video stream?
            if (packet->stream_index == is->videoStream) {
                is->videoq.Put(packet);
            } else if (packet->stream_index == is->audioStream)
                is->audioq.Put(packet);
            else
                av_free_packet(packet);
        }

        is->quit = 1;
        is->DecodeFinished();
    }

private:
    VideoState *is;
};

#pragma pack()


///////////////////////////////////////////////////////////////////////////////
//
// VideoThread
//

class VideoThread : public WorkerThread {
public:
    VideoThread(
        VideoState* is) :
            is(is)
    {
    }
protected:
    virtual void thread_proc()
    {
        AVPacket pkt1;
        AVPacket *packet = &pkt1;
        int len1, frameFinished;
        AVFrame *pFrame;
        double pts;
        int64_t pts_int;

        pFrame = av_frame_alloc();

        for (;;) {
            if (is->videoq.Get(packet, 1) < 0) {
                // means we quit getting packets
                break;
            }

            pts_int = 0;

            // Decode video frame
            len1 = avcodec_decode_video2(is->video_st->codec, pFrame, &frameFinished, packet);

#ifdef SIMULATE_SLOW_CPU
            Sleep(VIDEO_DECODE_STALL);
#endif

            // Did we get a video frame?
            if (frameFinished) {
                pts_int = pFrame->best_effort_timestamp;

                if (pts_int == AV_NOPTS_VALUE) {
                    pts_int = 0;
                }
                pts = (double) pts_int * av_q2d(is->video_st->time_base);

                pts = synchronize_video(pFrame, pts);

                is->skip_frames_index += 1;
                if (is->skip_frames_index >= is->skip_frames) {
                    is->skip_frames_index -= std::max(is->skip_frames, 1.0F);

                    if (queue_picture(pFrame, pts, packet->pos) < 0) {
                        av_free_packet(packet);
                        break;
                    }
                }
#ifdef _DEBUG
                else {
                    Warning(("ffmpeg: dropped decoded frame sfi:%g sf:%g pts:%g pktpts:%g\n", is->skip_frames_index, is->skip_frames, (double) pts, packet->pts));
                }
#endif
            }
            av_free_packet(packet);
        }
        av_free(pFrame);

        is->VideoFinished();
    }

private:
    double synchronize_video(AVFrame *src_frame, double pts)
    {
        double frame_delay;

        if (pts != 0) {
            /* if we have pts, set video clock to it */
            is->video_clock = pts;
        } else {
            /* if we aren't given a pts, set it to the clock */
            pts = is->video_clock;
        }
        /* update the video clock */
        frame_delay = av_q2d(is->video_st->codec->time_base);
        /* if we are repeating a frame, adjust clock accordingly */
        frame_delay += src_frame->repeat_pict * (frame_delay * 0.5);
        is->video_clock += frame_delay;
        return pts;
    }

    int queue_picture(AVFrame *pFrame, double pts, int64_t pos)
    {
        VideoState::VideoPicture *vp;

        // wait until we have space for a new pic
        is->pictq_mutex.lock();
        if (is->pictq_size >= VIDEO_PICTURE_QUEUE_SIZE && !is->refresh)
            is->skip_frames = std::max(1.0F - FRAME_SKIP_FACTOR, is->skip_frames * (1.0F - FRAME_SKIP_FACTOR));
        while (is->pictq_size >= VIDEO_PICTURE_QUEUE_SIZE &&	!is->quit) {
            SDL_CondWait(is->pictq_cond, is->pictq_mutex);
        }
        is->pictq_mutex.unlock();

        if (is->quit)
            return -1;

        // windex is set to 0 initially
        vp = &is->pictq[is->pictq_windex];

        ILGVideoDecoderHost::sLockResult lock;
        if (!is->pOuter->m_pHostIface->LockBuffer(vp->bmp, lock))
            return -1;

        ILGVideoDecoderHost::sFrameFormat fmt;
        is->pOuter->m_pHostIface->GetFrameFormat(fmt);

        uint8 *data[] = { (uint8*) lock.buffer, NULL, NULL };
        int stride[] = { lock.pitch, 0, 0 };

        is->img_convert_ctx = sws_getCachedContext(is->img_convert_ctx,
            is->video_st->codec->width, is->video_st->codec->height, is->video_st->codec->pix_fmt,
            fmt.width, fmt.height, is->pict_pix_fmt,
            sws_flags, NULL, NULL, NULL);

        sws_scale(is->img_convert_ctx, pFrame->data,
            pFrame->linesize, 0,
            is->video_st->codec->height, data, stride);

        is->pOuter->m_pHostIface->UnlockBuffer(vp->bmp);

        vp->pts = pts;
        vp->pos = pos;

        // now we inform our display thread that we have a pic ready
        if (++is->pictq_windex == VIDEO_PICTURE_QUEUE_SIZE) {
            is->pictq_windex = 0;
        }
        is->pictq_mutex.lock();
        vp->target_clock = is->compute_target_time(vp->pts);
        is->pictq_size++;
        is->pictq_mutex.unlock();

        return 0;
    }

    VideoState *is;
};


///////////////////////////////////////////////////////////////////////////////
//
// VideoState non-inline functions
//

BOOL VideoState::Play()
{
    decode_finished = 0;
    video_finished = 0;
    audio_finished = 0;

    videoq.Finished(0);
    audioq.Finished(0);
    videoq.Quit(0);
    audioq.Quit(0);
    quit = 0;

    audio_clock = 0;
    audio_diff_cum = 0;
    frame_last_pts = 0;
    video_clock = 0;
    video_current_pts = 0;
    pictq_size = 0;
    pictq_rindex = 0;
    pictq_windex = 0;

    audio_buf_size = 0;
    audio_buf_index = 0;

    /* averaging filter for audio sync */
    audio_diff_avg_coef = exp(log(0.01 / AUDIO_DIFF_AVG_NB));
    audio_diff_avg_count = 0;
    /* Correct audio only if larger error than this */
    if (audio_st)
        audio_diff_threshold = 2.0 * AUDIO_BUFFER_SIZE / (double) audio_st->codec->sample_rate;
    else
        audio_diff_threshold = 0;

    memset(&audio_pkt, 0, sizeof(audio_pkt));
    memset(&audio_pkt2, 0, sizeof(audio_pkt2));

    const int64_t curtime = av_gettime();

    frame_timer = (double) curtime / 1000000.0;
    frame_last_delay = 40e-3;
    video_current_pts_drift = 0;
    video_current_pos = 0;
    video_current_pts_time = curtime;

    external_clock_time = curtime;
    external_clock = 0;
    skip_frames = 0;
    skip_frames_index = 0;

    Assert_(video_tid == NULL);
    video_tid = new VideoThread(this);
    if (!video_tid->create()) {
        AssertMsg(FALSE, "VideoThread::Create");
    }

    schedule_refresh(40);

    Assert_(parse_tid == NULL);
    parse_tid = new DecodeThread(this);
    if (!parse_tid->create()) {
        AssertMsg(FALSE, "DecodeThread::Create");
    }

    return (parse_tid != 0);
}

int VideoState::stream_component_open(int stream_index)
{
    AVCodecContext *codecCtx;
    AVCodec *codec;

    if (stream_index < 0 || stream_index >= (int) pFormatCtx->nb_streams) {
        return -1;
    }

    // Get a pointer to the codec context for the video stream
    codecCtx = pFormatCtx->streams[stream_index]->codec;

    codec = avcodec_find_decoder(codecCtx->codec_id);

#ifdef _DEBUG
    codecCtx->debug = FF_DEBUG_BUGS |/*FF_DEBUG_VIS_MB_TYPE|*/FF_DEBUG_ER |/*FF_DEBUG_SKIP|FF_DEBUG_PICT_INFO|*/FF_DEBUG_PTS;
#endif

    if (!codec || (avcodec_open2(codecCtx, codec, NULL) < 0)) {
        AssertMsg(FALSE, "Unsupported codec!\n");
        return -1;
    }

    pFormatCtx->streams[stream_index]->discard = AVDISCARD_DEFAULT;

    switch (codecCtx->codec_type) {
    case AVMEDIA_TYPE_AUDIO:
    {
        // we want 16-bit samples out from the movie (they'll be converted if necessary)
        audio_src_fmt = AV_SAMPLE_FMT_S16;

        if (!pOuter->m_pHostIface->CreateAudioBuffer(codecCtx->sample_rate, codecCtx->channels, AUDIO_BUFFER_SIZE))
            break;

        audioStream = stream_index;
        audio_st = pFormatCtx->streams[stream_index];

        memset(&audio_pkt, 0, sizeof(audio_pkt));
    }
        break;

    case AVMEDIA_TYPE_VIDEO:
        videoStream = stream_index;
        video_st = pFormatCtx->streams[stream_index];
        break;

    default:
        break;
    }
    return 0;
}


void VideoState::Stop()
{
    quit = 1;

    videoq.Quit();
    audioq.Quit();

    SDL_CondSignal(pictq_cond);

    if (video_tid)
        video_tid->wait_for_close();
    delete video_tid;
    video_tid = 0;

    if (parse_tid)
        parse_tid->wait_for_close();
    delete parse_tid;
    parse_tid = 0;
}


///////////////////////////////////////////////////////////////////////////////
//
// cLGVideoDecoder implementaion
//

cLGVideoDecoder::cLGVideoDecoder(ILGVideoDecoderHost *pHostIface)
: m_pHostIface(pHostIface),
is(NULL)
{
}

cLGVideoDecoder::~cLGVideoDecoder()
{
}

STDMETHODIMP_(void) cLGVideoDecoder::Destroy()
{
    Stop();

    delete this;
}

STDMETHODIMP_(BOOL) cLGVideoDecoder::Start()
{
    if (!is)
        return FALSE;

    is->av_sync_type = VideoState::DEFAULT_AV_SYNC_TYPE;

    return is->Play();
}

STDMETHODIMP_(BOOL) cLGVideoDecoder::IsFinished()
{
    return is ? is->quit : TRUE;
}

STDMETHODIMP_(BOOL) cLGVideoDecoder::IsVideoFrameAvailable()
{
    is->refresh = 1;

    if (is->video_st && is->pictq_size) {
        // TODO: should we check if it's still to early to display frame and return FALSE?
        return TRUE;
    }

    // BBi
    if (is->subs.has_subtitles()) {
        if (is->audio_finished == 0 || is->video_finished == 0)
            return TRUE;
    }
    // BBi

    return FALSE;
}

STDMETHODIMP_(void) cLGVideoDecoder::RequestVideoFrame()
{
    if (is && is->refresh) {
        is->video_refresh_timer();
        is->refresh = 0;
    }

    // BBi
    if (is != NULL && is->subs.has_subtitles()) {
        if (is->audio_finished == 0 || is->video_finished == 0) {
            double pts = is->get_master_clock();

            if (is->subs.check_subtitle(pts))
                is->subs.refresh_subtitle = true;
        }

        if (is->subs.refresh_video || is->subs.refresh_subtitle) {
            is->pOuter->m_pHostIface->BeginVideoFrame(is->pictq[is->pictq_rindex].bmp);

            if (is->subs.refresh_video) {
                // update queue for next picture!
                if (++is->pictq_rindex == VIDEO_PICTURE_QUEUE_SIZE)
                    is->pictq_rindex = 0;

                is->pictq_mutex.lock();
                is->pictq_size--;
                ::SDL_CondSignal(is->pictq_cond);
                is->pictq_mutex.unlock();
            }

            is->pOuter->m_pHostIface->EndVideoFrame();

            is->subs.refresh_video = false;
            is->subs.refresh_subtitle = false;
        }
    }
    // BBi
}

STDMETHODIMP_(void) cLGVideoDecoder::RequestAudio(unsigned int len)
{
    if (is)
        is->audio_request(len);
}

//

void cLGVideoDecoder::Stop()
{
    if (is) {
        is->Stop();
        is->Close();

        delete is;
        is = NULL;
    }

    FFmpeg::Shutdown();
}

BOOL cLGVideoDecoder::Init(const char *filename)
{
    char buf[16] = { 0, };

    sws_flags = SWS_BICUBIC;
    if (m_pHostIface->GetConfigValue("movie_sw_scale_quality", buf, sizeof(buf))) {
        sws_flags = atoi(buf);

        if (sws_flags < 0)
            sws_flags = 0;
        else if (sws_flags > 6)
            sws_flags = 6;

        switch (sws_flags) {
        case 0: sws_flags = SWS_POINT; break;
        case 1: sws_flags = SWS_FAST_BILINEAR; break;
        case 2: sws_flags = SWS_BILINEAR; break;
        case 3: sws_flags = SWS_BICUBLIN; break;
        case 4: sws_flags = SWS_BICUBIC; break;
        case 5: sws_flags = SWS_SPLINE; break;
        case 6: sws_flags = SWS_SINC; break;
        default:
            sws_flags = SWS_BICUBIC;
        }
    }

    if (!FFmpeg::Init(this))
        return FALSE;

    avcodec_register_all();

    av_register_all();

    is = new VideoState(this);

    //

    ILGVideoDecoderHost::sFrameFormat fmt;
    m_pHostIface->GetFrameFormat(fmt);

    PixelFormat pixformat = PIX_FMT_NONE;

    if (fmt.bpp == 32) {
        if (fmt.gmask == 0xFF0000) // && bmask.alpha == 0xFF
        {
            if (fmt.rmask == 0xFF00 && fmt.bmask == 0xFF000000)
                pixformat = PIX_FMT_ARGB;
            else if (fmt.rmask == 0xFF000000 && fmt.bmask == 0xFF00)
                pixformat = PIX_FMT_ABGR;
        } else if (fmt.gmask == 0xFF00) // && bmask.alpha == 0xFF000000
        {
            if (fmt.rmask == 0xFF && fmt.bmask == 0xFF0000)
                pixformat = PIX_FMT_RGBA;
            else if (fmt.rmask == 0xFF0000 && fmt.bmask == 0xFF)
                pixformat = PIX_FMT_BGRA;
        }
    } else if (fmt.bpp == 16) {
        if (fmt.gmask == 0x7E0) {
            if (fmt.bmask == 0x1F)
                pixformat = PIX_FMT_RGB565LE;
            else
                pixformat = PIX_FMT_BGR565LE;
        }
    }

    if (pixformat == PIX_FMT_NONE) {
        // failed to find suitable pixel format (probably in 8-bit mode)
        Assert_(pixformat != PIX_FMT_NONE);
        Stop();
        return FALSE;
    }

    if (!is->Open(filename, fmt.width, fmt.height, pixformat)) {
        Stop();
        return FALSE;
    }

    return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
//
// DLL interface
//

extern "C" ILGVideoDecoder* CreateLGVideoDecoder(ILGVideoDecoderHost *pHostIface, const char *filename)
{
    if (!pHostIface || !filename)
        return NULL;

    cLGVideoDecoder *p = new cLGVideoDecoder(pHostIface);

    if (!p->Init(filename)) {
        delete p;
        return NULL;
    }

    return p;
}
