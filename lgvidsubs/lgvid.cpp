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


// load ffmpeg as a DLL
#ifdef _MSC_VER
#define FFMPEG_DLL
#endif


#define __STDC_CONSTANT_MACROS

#include <windows.h>
#include <process.h>

extern "C"
{
#pragma pack(8) // must have same aligment as the ffmpeg libs
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavcodec/audioconvert.h>
#pragma pack()
}

#include "lgviddecoder.h"

//BBi
#include <fstream>

#include "bbi_srt_parser.h"
#include "bbi_subtitles.h"
//BBi


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


#define AUDIO_BUFFER_SIZE 16*1024
//#define MAX_AUDIOQ_SIZE (5 * 256 * 1024)
//#define MAX_VIDEOQ_SIZE (5 * 256 * 1024)
#define MIN_AUDIOQ_SIZE (20 * 16 * 1024)
#define MIN_FRAMES 5
#define AV_SYNC_THRESHOLD 0.01
#define AV_NOSYNC_THRESHOLD 10.0
#define SAMPLE_CORRECTION_PERCENT_MAX 10
#define AUDIO_DIFF_AVG_NB 20
#define FF_ALLOC_EVENT   (SDL_USEREVENT)
#define FF_REFRESH_EVENT (SDL_USEREVENT + 1)
#define FF_QUIT_EVENT (SDL_USEREVENT + 2)
#define VIDEO_PICTURE_QUEUE_SIZE 2
#define DEFAULT_AV_SYNC_TYPE AV_SYNC_EXTERNAL_MASTER

static int sws_flags = SWS_BICUBIC;


#ifdef _DEBUG
// uncomment to simulate movie playback on slower CPU
//#define SIMULATE_SLOW_CPU

// simulated extra time (ms) for an audio packet to decode
#define AUDIO_DECODE_STALL (1000/60)
// simluated extra time (ms) for a video frame to decode
#define VIDEO_DECODE_STALL (1000/25)
#endif

///////////////////////////////////////////////////////////////////////////////

struct VideoState;

class cLGVideoDecoder : public ILGVideoDecoder
{
public:
	ILGVideoDecoderHost *m_pHostIface;

	VideoState *is;

public:
	cLGVideoDecoder(ILGVideoDecoderHost *pHostIface);
	virtual ~cLGVideoDecoder();

	// ILGVideoDecoder interface implementation
	STDMETHOD_(void,Destroy)();
	STDMETHOD_(BOOL,Start)();
	STDMETHOD_(BOOL,IsFinished)();
	STDMETHOD_(BOOL,IsVideoFrameAvailable)();
	STDMETHOD_(void,RequestVideoFrame)();
	STDMETHOD_(void,RequestAudio)(unsigned int len);

	void Stop();
	BOOL Init(const char *filename);
};


///////////////////////////////////////////////////////////////////////////////
//
// FFmpeg dynamic lib API
//

namespace FFmpeg
{
	void* (*av_malloc)(FF_INTERNAL_MEM_TYPE size);
	void (*av_freep)(void *ptr);
	int64_t (*av_gettime)(void);
	void (*av_free_packet)(AVPacket *pkt);
	int (*av_read_frame)(AVFormatContext *s, AVPacket *pkt);
	void (*av_init_packet)(AVPacket *pkt);
	int (*av_dup_packet)(AVPacket *pkt);
	void (*av_free)(void *ptr);
	//int (*av_get_bits_per_sample)(enum CodecID codec_id);
	int (*av_get_bits_per_sample_fmt)(enum AVSampleFormat sample_fmt);
	const char* (*av_get_sample_fmt_name)(enum AVSampleFormat sample_fmt);
	void (*av_register_all)(void);
	void (*av_close_input_file)(AVFormatContext *s);
	int (*av_find_stream_info)(AVFormatContext *ic);
	//int (*av_open_input_file)(AVFormatContext **ic_ptr, const char *filename, AVInputFormat *fmt, int buf_size, AVFormatParameters *ap);
	int (*av_find_best_stream)(AVFormatContext *ic, enum AVMediaType type, int wanted_stream_nb, int related_stream, AVCodec **decoder_ret, int flags);
	int (*av_open_input_stream)(AVFormatContext **ic_ptr, AVIOContext *pb, const char *filename, AVInputFormat *fmt, AVFormatParameters *ap);
	void (*av_close_input_stream)(AVFormatContext *s);
	int (*av_probe_input_buffer)(AVIOContext *pb, AVInputFormat **fmt, const char *filename, void *logctx, unsigned int offset, unsigned int max_probe_size);
	int (*av_audio_convert)(AVAudioConvert *ctx, void * const out[6], const int out_stride[6], const void * const  in[6], const int  in_stride[6], int len);

	void (*av_log_set_callback)(void (*)(void*, int, const char*, va_list));
	void (*av_log_set_level)(int);
	void (*dump_format)(AVFormatContext *ic, int index, const char *url, int is_output);

	AVFormatContext* (*avformat_alloc_context)(void);
	void (*avformat_free_context)(AVFormatContext *s);
	AVAudioConvert* (*av_audio_convert_alloc)(enum AVSampleFormat out_fmt, int out_channels, enum AVSampleFormat in_fmt, int in_channels, const float *matrix, int flags);
	void (*av_audio_convert_free)(AVAudioConvert *ctx);

	void (*avcodec_init)(void);
	int (*avcodec_default_get_buffer)(AVCodecContext *s, AVFrame *pic);
	void (*avcodec_default_release_buffer)(AVCodecContext *s, AVFrame *pic);
	int (*avcodec_decode_video2)(AVCodecContext *avctx, AVFrame *picture, int *got_picture_ptr, AVPacket *avpkt);
	AVFrame* (*avcodec_alloc_frame)(void);
	int (*avcodec_open)(AVCodecContext *avctx, AVCodec *codec);
	AVCodec* (*avcodec_find_decoder)(enum CodecID id);
	int (*avcodec_decode_audio3)(AVCodecContext *avctx, int16_t *samples, int *frame_size_ptr, AVPacket *avpkt);
	int (*avcodec_close)(AVCodecContext *avctx);

	int (*sws_scale)(struct SwsContext *context, const uint8_t* const srcSlice[], const int srcStride[], int srcSliceY, int srcSliceH, uint8_t* const dst[], const int dstStride[]);
	void (*sws_freeContext)(struct SwsContext *swsContext);
	//struct SwsContext* (*sws_getContext)(int srcW, int srcH, enum PixelFormat srcFormat, int dstW, int dstH, enum PixelFormat dstFormat, int flags, SwsFilter *srcFilter, SwsFilter *dstFilter, const double *param);
	struct SwsContext* (*sws_getCachedContext)(struct SwsContext *context, int srcW, int srcH, enum PixelFormat srcFormat, int dstW, int dstH, enum PixelFormat dstFormat, int flags, SwsFilter *srcFilter, SwsFilter *dstFilter, const double *param);

	AVIOContext* (*avio_alloc_context)(unsigned char *buffer, int buffer_size, int write_flag, void *opaque, int (*read_packet)(void *opaque, uint8_t *buf, int buf_size), int (*write_packet)(void *opaque, uint8_t *buf, int buf_size), int64_t (*seek)(void *opaque, int64_t offset, int whence));

	//

#ifdef FFMPEG_DLL
	HMODULE hDll = NULL;
#endif
	cLGVideoDecoder *pOuter = NULL;

	//

#ifdef FFMPEG_DLL
	#define INIT_FF_CALL(_name) \
		if (((void*&)_name = GetProcAddress(hDll, #_name)) == NULL) \
		{ \
			mprintf("failed to resolve FFmpeg call %s", #_name); \
			goto fail;\
		}
#else
	#define INIT_FF_CALL(_name) _name = :: _name;
#endif

	static void mprintf(const char *fmt, ...)
	{
		char buf[1536];

		if (pOuter)
		{
			va_list ap;
			va_start(ap, fmt);
			vsprintf(buf, fmt, ap);
			va_end(ap);

			pOuter->m_pHostIface->LogPrint(buf);
		}
	}

	static void ffmpeg_log_callback(void *ptr, int level, const char *fmt, va_list vl)
	{
		char s[1024];

		vsprintf(s, fmt, vl);

		AVClass *avc = ptr ? *(AVClass **)ptr : NULL;

		if (ptr)
		{
			if (avc)
				mprintf("FFMPEG> %s[%s]: %s", avc->class_name, avc->item_name(ptr), s);
			else
				mprintf("FFMPEG> %s: %s", avc->class_name, s);
		}
		else
			mprintf("FFMPEG> %s", s);
	}

	void Shutdown()
	{
#ifdef FFMPEG_DLL
		if (hDll)
		{
			FreeLibrary(hDll);
			hDll = NULL;
		}
#endif

		pOuter = NULL;
	}

	BOOL Init(cLGVideoDecoder *pOuter_)
	{
		pOuter = pOuter_;

#ifdef FFMPEG_DLL
		if (hDll)
		{
			return TRUE;
		}

		hDll = LoadLibrary("ffmpeg.dll");
		if (!hDll)
		{
			// NOTE: could show message box (once) here, OTOH message boxes and fullscreen is just a bad combo
			mprintf("Failed to find/load ffmpeg.dll, no movie playback possible");
			return FALSE;
		}
#endif

		INIT_FF_CALL(av_malloc);
		INIT_FF_CALL(av_freep);
		INIT_FF_CALL(av_gettime);
		INIT_FF_CALL(av_free_packet);
		INIT_FF_CALL(av_read_frame);
		INIT_FF_CALL(av_init_packet);
		INIT_FF_CALL(av_dup_packet);
		INIT_FF_CALL(av_free);
		//INIT_FF_CALL(av_get_bits_per_sample);
		INIT_FF_CALL(av_get_bits_per_sample_fmt);
		INIT_FF_CALL(av_get_sample_fmt_name);
		INIT_FF_CALL(av_register_all);
		INIT_FF_CALL(av_close_input_file);
		INIT_FF_CALL(av_find_stream_info);
		//INIT_FF_CALL(av_open_input_file);
		INIT_FF_CALL(av_find_best_stream);
		INIT_FF_CALL(av_open_input_stream);
		INIT_FF_CALL(av_close_input_stream);
		INIT_FF_CALL(av_probe_input_buffer);
		INIT_FF_CALL(av_audio_convert);

		INIT_FF_CALL(av_log_set_callback);
		INIT_FF_CALL(av_log_set_level);
		INIT_FF_CALL(dump_format);

		INIT_FF_CALL(avformat_alloc_context);
		INIT_FF_CALL(avformat_free_context);
		INIT_FF_CALL(av_audio_convert_alloc);
		INIT_FF_CALL(av_audio_convert_free);

		INIT_FF_CALL(avcodec_init);
		INIT_FF_CALL(avcodec_default_get_buffer);
		INIT_FF_CALL(avcodec_default_release_buffer);
		INIT_FF_CALL(avcodec_decode_video2);
		INIT_FF_CALL(avcodec_alloc_frame);
		INIT_FF_CALL(avcodec_open);
		INIT_FF_CALL(avcodec_find_decoder);
		INIT_FF_CALL(avcodec_decode_audio3);
		INIT_FF_CALL(avcodec_close);

		INIT_FF_CALL(sws_scale);
		INIT_FF_CALL(sws_freeContext);
		//INIT_FF_CALL(sws_getContext);
		INIT_FF_CALL(sws_getCachedContext);

		INIT_FF_CALL(avio_alloc_context);

#ifndef SHIP
		if ( pOuter->m_pHostIface->GetConfigValue("ffmpeg_spew", NULL, 0) )
		{
			av_log_set_callback(ffmpeg_log_callback);

			av_log_set_level(99);
		}
#endif

		return TRUE;

#ifdef FFMPEG_DLL
fail:
		mprintf("WARNING: cannot play movie, wrong version of \"ffmpeg.dll\"");

		Shutdown();

		return FALSE;
#endif
	}

	//
	// I/O interface
	//

	struct IOContext
	{
		void *pStream;
		ulong size;

		IOContext(void *str) { pStream = str; size = pOuter->m_pHostIface->FileSize(str); }
	};

	static int Read(void *opaque, uint8_t *buf, int buf_size)
	{
		IOContext *io = (IOContext*)opaque;
		return pOuter->m_pHostIface->FileRead(io->pStream, buf, buf_size);
	}

	static int64_t Seek(void *opaque, int64_t offset, int whence)
	{
		IOContext *io = (IOContext*)opaque;
		return (whence == AVSEEK_SIZE) ? (QWORD)io->size : pOuter->m_pHostIface->FileSeek(io->pStream, offset, whence);
	}


	int OpenFile(AVFormatContext **ic_ptr, const char *filename, AVInputFormat *fmt, int buf_size, AVFormatParameters *ap)
	{
		//return av_open_input_file(ic_ptr, filename, fmt, buf_size, ap);

		void *pStream = pOuter->m_pHostIface->FileOpen(filename);
		if (!pStream)
		{
			mprintf("failed to open file '%s'", filename);
			return -1;
		}

		int ret = 0;

		IOContext *ctxt = new IOContext(pStream);

		AVIOContext *pb = avio_alloc_context(NULL, 0, 0, ctxt, Read, NULL/*Write*/, Seek);

		if (!fmt)
		{
			// determine format
			av_probe_input_buffer(pb, &fmt, filename, ap && ap->prealloced_context ? *ic_ptr : NULL, 0, 0);

			if (!fmt)
			{
				Warning(("failed to determine file format '%s'\n", filename));
				ret = -1;
				goto fail;
			}
		}

		ret = av_open_input_stream(ic_ptr, pb, filename, fmt, ap);

		if (ret)
		{
fail:
			pOuter->m_pHostIface->FileClose(pStream);
			delete ctxt;
			av_free(pb);
		}

		return ret;
	}

	void CloseFile(AVFormatContext *s)
	{
		Assert_(s != NULL);

		//av_close_input_file(s);

		AVIOContext *pb = s->pb;

		av_close_input_stream(s);

		if (pb)
		{
			if (pb->opaque)
			{
				IOContext *io = (IOContext*)pb->opaque;

				if (io->pStream)
					pOuter->m_pHostIface->FileClose(io->pStream);

				delete io;
				pb->opaque = NULL;
			}

			av_free(pb);
		}
	}
};


#pragma pack(8)


///////////////////////////////////////////////////////////////////////////////
// Thread Utilities

class cThreadLock
{
public:
	cThreadLock() { InitializeCriticalSection(&m_CritSec); }
	~cThreadLock() { DeleteCriticalSection(&m_CritSec); }
	void Lock() { EnterCriticalSection(&m_CritSec); }
	void Unlock() { LeaveCriticalSection(&m_CritSec); }
private:
	CRITICAL_SECTION m_CritSec;
};

class cAutoLock
{
public:
    cAutoLock(cThreadLock &lock) : m_lock(lock) { m_lock.Lock(); }
    ~cAutoLock() { m_lock.Unlock(); }
private:
    cThreadLock &m_lock;
};

class cThreadSyncObject
{
public:
	~cThreadSyncObject() { if (m_hSyncObject) CloseHandle(m_hSyncObject); }
	BOOL operator!() const { return !m_hSyncObject; }
	operator HANDLE () { return m_hSyncObject; }
	BOOL Wait(DWORD dwTimeout = INFINITE) { return WaitForSingleObject(m_hSyncObject, dwTimeout) == WAIT_OBJECT_0; }
protected:
	cThreadSyncObject() : m_hSyncObject(NULL) {}
	HANDLE m_hSyncObject;
};

class cThreadEvent : public cThreadSyncObject
{
public:
	cThreadEvent(BOOL fManualReset = FALSE) { m_hSyncObject = CreateEvent(NULL, fManualReset, FALSE, NULL); }
	BOOL Set() { return SetEvent(m_hSyncObject); }
	BOOL Reset() { return ResetEvent(m_hSyncObject); }
	BOOL Pulse() { return PulseEvent(m_hSyncObject); }
	BOOL Check() { return Wait(0); }
};

class cThreadSemaphore : public cThreadSyncObject
{
public:
	cThreadSemaphore(long initialValue, long maxValue) { m_hSyncObject = CreateSemaphore(NULL, initialValue, maxValue, NULL); }
	BOOL Release(long releaseCount = 1, long * pPreviousCount = NULL) { return ReleaseSemaphore(m_hSyncObject, releaseCount, pPreviousCount); }
};

class cThreadMutex : public cThreadSyncObject
{
public:
	cThreadMutex(BOOL fEstablishInitialOwnership = FALSE) { m_hSyncObject = CreateMutex(NULL, fEstablishInitialOwnership, NULL); }
	BOOL Release() { return ReleaseMutex(m_hSyncObject); }
};


class cWorkerThread
{
public:
	cWorkerThread() : m_hThread(NULL), m_EventSend(TRUE) {}
	virtual ~cWorkerThread() { if (m_hThread) WaitForClose(); }
	BOOL Create()
	{
		unsigned int threadid;
		cAutoLock lock(m_Lock);
		if (ThreadExists())
		{
			AssertMsg(FALSE, "thread already created");
			return FALSE;
		}
		m_hThread = (HANDLE) _beginthreadex(NULL, 0, cWorkerThread::InitialThreadProc, this, 0, &threadid);
		AssertMsg1(m_hThread, "create thread failed (%x)", GetLastError());
		if (!m_EventComplete.Wait(10000))
		{
			AssertMsg(FALSE, "timed out waiting for worker thread to init");
		}
		return m_hThread != NULL;
	}
	void WaitForClose(DWORD dwErrorTimeout = 10000)
	{
		if (!m_hThread)
			return;
		if (WaitForSingleObject(m_hThread, dwErrorTimeout) == WAIT_TIMEOUT)
		{
			AssertMsg(FALSE, "timed out waiting for worker thread to close");
		}
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
	BOOL ThreadExists() { DWORD dwExitCode; return (m_hThread && GetExitCodeThread(m_hThread, &dwExitCode) && dwExitCode == STILL_ACTIVE); }
	int GetPriority() const { return GetThreadPriority(m_hThread); }
	BOOL SetPriority(int priority) { return SetThreadPriority(m_hThread, priority); }
	DWORD Suspend() { return SuspendThread(m_hThread); }
	DWORD Resume() { return ResumeThread(m_hThread); }
	BOOL Terminate(DWORD dwExitCode) { return TerminateThread(m_hThread, dwExitCode); }
	DWORD CallWorker(DWORD dw, BOOL fBoostWorkerPriorityToMaster = TRUE) { return Call(dw, fBoostWorkerPriorityToMaster); }
	DWORD CallMaster(DWORD dw) { return Call(dw, FALSE); }
	DWORD WaitForCall() { m_EventSend.Wait(); return m_dwParam; }
	BOOL PeekCall(DWORD *pParam = NULL) { if (!m_EventSend.Check()) return FALSE; if (pParam) *pParam = m_dwParam; return TRUE; }
	void Reply(DWORD dw) { m_dwParam = kInvalidCallParam; m_dwReturnVal = dw; m_EventSend.Reset(); m_EventComplete.Set(); }
	HANDLE GetCallHandle() { return m_EventSend; }
	DWORD GetCallParam() const { return m_dwParam; }
protected:
	enum { kInvalidCallParam = 0xffffffff };
	virtual DWORD ThreadProc() = 0;
	cThreadLock m_Lock;
private:
	DWORD Call(DWORD dwParam, BOOL fBoostPriority)
	{
		AssertMsg(!m_EventSend.Check(), "nested inter-thread call");
		cAutoLock lock(m_Lock);
		if (!ThreadExists())
			return E_FAIL;
		int iInitialPriority;
		if (fBoostPriority)
		{
			iInitialPriority = GetPriority();
			const int iNewPriority = GetThreadPriority(GetCurrentThread());
			if (iNewPriority > iInitialPriority)
				SetPriority(iNewPriority);
		}
		m_dwParam = dwParam;
		m_EventSend.Set();
		m_EventComplete.Wait();
		if (fBoostPriority)
			SetPriority(iInitialPriority);
		return m_dwReturnVal;
	}
	static unsigned __stdcall InitialThreadProc(LPVOID pv)
	{
		cWorkerThread * pThread = (cWorkerThread *) pv;
		pThread->m_EventComplete.Set();
		return pThread->ThreadProc();
	}
	HANDLE          m_hThread;
	cThreadEvent    m_EventSend;
	cThreadEvent    m_EventComplete;
	DWORD           m_dwParam;
	DWORD           m_dwReturnVal;
};

///////////////////////////////////////////////////////////////////////////////
// SDL_Cond

struct SDL_cond
{
	cThreadMutex lock;
	int waiting;
	int signals;
	cThreadSemaphore wait_sem;
	cThreadSemaphore wait_done;

	SDL_cond() : wait_sem(0, 32*1024), wait_done(0, 32*1024)
	{
		waiting = signals = 0;
	}
};


int SDL_CondSignal(SDL_cond &cond)
{
	cond.lock.Wait();

    if (cond.waiting > cond.signals) {
        ++cond.signals;
		cond.wait_sem.Release();
		cond.lock.Release();
		cond.wait_done.Wait();
    } else {
		cond.lock.Release();
    }

    return 0;
}

int SDL_CondWaitTimeout(SDL_cond &cond, cThreadMutex &mutex, uint32 ms)
{
    int retval;

	cond.lock.Wait();
    ++cond.waiting;
	cond.lock.Release();

	mutex.Release();

	retval = !cond.wait_sem.Wait(ms);

	cond.lock.Wait();

    if (cond.signals > 0) {
        if (retval > 0)
			cond.wait_sem.Wait();
		cond.wait_done.Release();
        --cond.signals;
    }

    --cond.waiting;
	cond.lock.Release();

	mutex.Wait();

    return retval;
}

inline int SDL_CondWait(SDL_cond &cond, cThreadMutex &mutex)
{
	return SDL_CondWaitTimeout(cond, mutex, ~0);
}


///////////////////////////////////////////////////////////////////////////////
//
// PacketQueue
//

struct PacketQueue
{
	PacketQueue() : first_pkt(0), last_pkt(0), nb_packets(0), size(0), quit(0), finished(0) {};
	~PacketQueue()
	{
		Flush();
	}


	AVPacketList *first_pkt, *last_pkt;
	int nb_packets;
	int size;
	cThreadMutex mutex;
	SDL_cond cond;

	int Put(AVPacket *pkt)
	{
		Assert_(!finished && !quit);

		AVPacketList *pkt1;
		if(FFmpeg::av_dup_packet(pkt) < 0) {
			return -1;
		}

		pkt1 = (AVPacketList*)FFmpeg::av_malloc(sizeof(AVPacketList));
		if (!pkt1)
			return -1;
		pkt1->pkt = *pkt;
		pkt1->next = NULL;

		mutex.Wait();

		if (!last_pkt)
			first_pkt = pkt1;
		else
			last_pkt->next = pkt1;
		last_pkt = pkt1;
		nb_packets++;
		size += pkt1->pkt.size + sizeof(*pkt1);

		SDL_CondSignal(cond);

		mutex.Release();
		return 0;
	}

	int Get(AVPacket *pkt, int block) 
	{
		AVPacketList *pkt1;
		int ret;

		mutex.Wait();

		for(;;) {
			if(quit) {
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
				FFmpeg::av_free(pkt1);
				ret = 1;
				break;
			} else if (!block) {
				ret = 0;
				break;
			} else {
				// If finished and queue is empty, let receiver know
				if(finished) {
					ret = -1;
					break;
				}

				SDL_CondWait(cond, mutex);
			}
		}
		mutex.Release();
		return ret;
	}
	
	void Flush() 
	{
		AVPacketList *pkt, *pkt1;

		mutex.Wait();
		for(pkt = first_pkt; pkt != NULL; pkt = pkt1) {
			pkt1 = pkt->next;
			FFmpeg::av_free_packet(&pkt->pkt);
			FFmpeg::av_freep(&pkt);
		}
		last_pkt = NULL;
		first_pkt = NULL;
		nb_packets = 0;
		size = 0;
		mutex.Release();
	}

	void Quit(int ret = 1)
	{
		mutex.Wait();
		quit = ret;

		SDL_CondSignal(cond);

		mutex.Release();
	}
	
	void Finished(int ret = 1)
	{
		mutex.Wait();
		finished = ret;

		SDL_CondSignal(cond);

		mutex.Release();
	}

	void Depleted()
	{
		mutex.Wait();

		finished = 1;
		if (!first_pkt)
			SDL_CondSignal(cond);

		mutex.Release();
	}
private:
	int quit;
	int finished;
};


///////////////////////////////////////////////////////////////////////////////
//
// VideoState
//

//int our_get_buffer(struct AVCodecContext *c, AVFrame *pic);
//void our_release_buffer(struct AVCodecContext *c, AVFrame *pic);

struct VideoState 
{	
	struct VideoPicture {
		void *bmp;
		double pts;				///<presentation time stamp for this picture
		double target_clock;	///<av_gettime() time at which this should be displayed ideally
		int64_t pos;			///<byte position in file
	};
	
	enum AV_SYNC
	{
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
	DECLARE_ALIGNED(16, uint8_t, audio_buf1[(AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2]);
	DECLARE_ALIGNED(16, uint8_t, audio_buf2[(AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2]);
	uint8_t         *audio_buf;
	unsigned int    audio_buf_size;
	unsigned int    audio_buf_index;
	AVPacket        audio_pkt;
	AVPacket        audio_pkt2;
    AVSampleFormat  audio_src_fmt;
    AVAudioConvert  *reformat_ctx;
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
	cThreadMutex    pictq_mutex;
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

    //BBi
    SubtitleList subtitles;
    //BBi

	VideoState(cLGVideoDecoder *pOuter_)
		: pOuter(pOuter_),
		pFormatCtx(0),
		videoStream(-1),
		audioStream(-1),
		av_sync_type(DEFAULT_AV_SYNC_TYPE),
		external_clock(0),
		external_clock_time(0),
		audio_clock(0),
		audio_st(0),
		audio_buf_size(0),
		audio_buf_index(0),
		audio_src_fmt(AV_SAMPLE_FMT_NONE),
		reformat_ctx(NULL),
		audio_diff_cum(0),
		audio_diff_avg_coef(0),
		audio_diff_threshold(0),
		audio_diff_avg_count(0),
		frame_timer(0),
		frame_last_pts(0),
		frame_last_delay(0),
		video_clock(0),
		video_current_pts(0),
		video_current_pts_drift(0),
		video_current_pos(0),
		video_current_pts_time(0),
		video_st(0),
		pictq_size(0),
		pictq_rindex(0),
		pictq_windex(0),
		parse_tid(0),
		video_tid(0),
		quit(0),
		img_convert_ctx(0),
		decode_finished(0),
		video_finished(0),
		audio_finished(0),
		skip_frames(0),
		skip_frames_index(0),
		refresh(0)

        //BBi
        , subtitles ()
        //BBi
	{
		audio_buf = audio_buf1;

		FFmpeg::av_init_packet(&audio_pkt);
		memset(pictq, 0, sizeof(pictq));

		//timer = CreateWaitableTimer(NULL, TRUE, NULL);
	}

	~VideoState()
	{
		//CloseHandle(timer);
		Close();
	}

	void schedule_refresh(int delay)
	{
		/*LARGE_INTEGER liDueTime;

		Assert_(delay > 0);

		// Per 100 nanosecond
		liDueTime.QuadPart = -delay*10000LL;

		SetWaitableTimer(
			timer,
			&liDueTime,
			0,
			NULL,
			NULL,
			FALSE
		);*/
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
			pts -= (double)hw_buf_size / bytes_per_sec;
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
		return video_current_pts_drift + FFmpeg::av_gettime() / 1000000.0;
	}

	double get_external_clock() const 
	{
		const int64_t ti = FFmpeg::av_gettime();
		return external_clock + ((ti - external_clock_time) * 1e-6);
	}

	double get_master_clock() const
	{
		double val;

		if (av_sync_type == AV_SYNC_VIDEO_MASTER)
		{
			if (video_st)
				val = get_video_clock();
			else
				val = get_audio_clock();
		}
		else if (av_sync_type == AV_SYNC_AUDIO_MASTER)
		{
			if (audio_st)
				val = get_audio_clock();
			else
				val = get_video_clock();
		}
		else
		{
			val = get_external_clock();
		}

		return val;
	}

	BOOL Open(const char *filename, int target_w, int target_h, PixelFormat dst_pix_fmt)
	{
		Close();

		pFormatCtx = FFmpeg::avformat_alloc_context();

		AVFormatParameters params, *ap = &params;

		memset(ap, 0, sizeof(*ap));
		ap->prealloced_context = 1;
		ap->time_base.den = 1;
		ap->time_base.num = 25;

		// Open video file
		if(FFmpeg::OpenFile(&pFormatCtx, filename, NULL, 0, ap)!=0)
			return FALSE; // Couldn't open file

		// Retrieve stream information
		if(FFmpeg::av_find_stream_info(pFormatCtx)<0)
			return FALSE; // Couldn't find stream information

		if(pFormatCtx->pb)
			pFormatCtx->pb->eof_reached = 0;

#if defined(_DEBUG) || defined(DEBUG)
		// Dump information about file onto standard error
		FFmpeg::dump_format(pFormatCtx, 0, filename, 0);
#endif

		// Find the first video stream

		int st_index[AVMEDIA_TYPE_NB];
		memset(st_index, -1, sizeof(st_index));

		for (unsigned int i=0; i<pFormatCtx->nb_streams; i++)
			pFormatCtx->streams[i]->discard = AVDISCARD_ALL;

        st_index[AVMEDIA_TYPE_VIDEO] = FFmpeg::av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
        st_index[AVMEDIA_TYPE_AUDIO] = FFmpeg::av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_AUDIO, -1, st_index[AVMEDIA_TYPE_VIDEO], NULL, 0);
        //st_index[AVMEDIA_TYPE_SUBTITLE] = FFmpeg::av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_SUBTITLE, -1, (st_index[AVMEDIA_TYPE_AUDIO] >= 0 ? st_index[AVMEDIA_TYPE_AUDIO] : st_index[AVMEDIA_TYPE_VIDEO]), NULL, 0);

		// Open streams

		if(st_index[AVMEDIA_TYPE_AUDIO] >= 0) {
			stream_component_open(st_index[AVMEDIA_TYPE_AUDIO]);
		}
		if(st_index[AVMEDIA_TYPE_VIDEO] >= 0) {
			stream_component_open(st_index[AVMEDIA_TYPE_VIDEO]);
		}   
		/*if (st_index[AVMEDIA_TYPE_SUBTITLE] >= 0) {
			stream_component_open(st_index[AVMEDIA_TYPE_SUBTITLE]);
		}*/

		if(videoStream < 0) {
			AssertMsg1(FALSE, "%s: could not open codecs\n", filename);
			return FALSE;
		}		

		// Init video scaler

		pict_pix_fmt = dst_pix_fmt;

		/* DEPRECATED, sws_getCachedContext is the new function
		int w = video_st->codec->width;
		int h = video_st->codec->height;
		img_convert_ctx = FFmpeg::sws_getContext(
			w, h, video_st->codec->pix_fmt, 
			target_w, target_h, dst_pix_fmt, 
			sws_flags, NULL, NULL, NULL);*/

		// Allocate frame buffer(s)
		for(int i = 0; i < VIDEO_PICTURE_QUEUE_SIZE; ++i)
			pictq[i].bmp = pOuter->m_pHostIface->CreateImageBuffer();

        //BBi
        std::string subFileName = filename;
        std::string::size_type dotPos = subFileName.rfind ('.');
        if (dotPos != std::string::npos)
            subFileName.erase (dotPos, subFileName.size () - dotPos);
        subFileName += ".srt";


        std::ifstream srtStream (subFileName.c_str ());

        if (srtStream.is_open ())
            subtitles = SrtParser::parse (srtStream);
        else
            subtitles.clear ();
        //BBi

		return TRUE;
	}

	void Stop();

	void Close()
	{
		Stop();

		if (reformat_ctx)
			FFmpeg::av_audio_convert_free(reformat_ctx);
		reformat_ctx = NULL;

		videoStream = -1;
		audioStream = -1;

		videoq.Flush();
		audioq.Flush();
        
		// Close the codec
		if(video_st)
			FFmpeg::avcodec_close(video_st->codec);
		video_st = NULL;

		if(audio_st)
			FFmpeg::avcodec_close(audio_st->codec);
		audio_st = NULL;

		// Close the video file
		if(pFormatCtx)
			FFmpeg::CloseFile(pFormatCtx);
		pFormatCtx = NULL;

		if(img_convert_ctx)
			FFmpeg::sws_freeContext(img_convert_ctx);
		img_convert_ctx = NULL;
		
		for(int i = 0; i < VIDEO_PICTURE_QUEUE_SIZE; ++i)
			pictq[i].bmp = NULL;
	}

	BOOL Play();

	void video_refresh_timer()
	{
		VideoPicture *vp;

		if(video_st) {
retry:
			if(pictq_size == 0) {
				schedule_refresh(1);
			} else {
				double time = FFmpeg::av_gettime()/1000000.0;
				double next_target;

				/* dequeue the picture */
				vp = &pictq[pictq_rindex];

				if(time < vp->target_clock)
				{
#ifdef _DEBUG
					//Warning(("ffmpeg: too early to display frame t:%g targt:%g\n", time, vp->target_clock));
#endif
					return;
				}

				/* update current video pts */
				video_current_pts = vp->pts;
				video_current_pts_drift = video_current_pts - time;
				video_current_pos = vp->pos;

				if(pictq_size > 1) {
					VideoPicture *nextvp= &pictq[(pictq_rindex+1)%VIDEO_PICTURE_QUEUE_SIZE];
					Assert_(nextvp->target_clock >= vp->target_clock);
					next_target= nextvp->target_clock;
				} else {
					next_target= vp->target_clock + video_clock - vp->pts; //FIXME pass durations cleanly
				}

				#define FRAME_SKIP_FACTOR 0.05
				/**/const BOOL framedrop = TRUE;

				if(framedrop && time > next_target) {
					skip_frames *= 1.0 + FRAME_SKIP_FACTOR;
					//if(pictq_size > 1 || time > next_target + 0.5) {
					if(pictq_size > 1 && time > next_target + 0.5) {
						/* update queue size and signal for next picture */
						if (++pictq_rindex == VIDEO_PICTURE_QUEUE_SIZE)
							pictq_rindex = 0;

#ifdef _DEBUG
						Warning(("ffmpeg: dropped display frame dt:%g q:%d sf:%g\n", next_target-time, pictq_size, skip_frames));
#endif

						pictq_mutex.Wait();
						pictq_size--;
						SDL_CondSignal(pictq_cond);
						pictq_mutex.Release();
						goto retry;
					}
				}

				// show the picture!

				pOuter->m_pHostIface->BeginVideoFrame(pictq[pictq_rindex].bmp);

				// update queue for next picture!
				if(++pictq_rindex == VIDEO_PICTURE_QUEUE_SIZE) {
					pictq_rindex = 0;
				}

				pictq_mutex.Wait();
				pictq_size--;
				SDL_CondSignal(pictq_cond);
				pictq_mutex.Release();

				pOuter->m_pHostIface->EndVideoFrame();
			}
		} else {
			schedule_refresh(100);
		}

		/*switch(av_sync_type)
		{
		case AV_SYNC_VIDEO_MASTER:
			if(video_finished)
				quit = 1;
			break;
		case AV_SYNC_AUDIO_MASTER:
			if(audio_finished)
				quit = 1;
			break;
		case AV_SYNC_EXTERNAL_MASTER:
			if(video_finished || audio_finished)
				quit = 1;
			break;
		}*/
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

		if (!len)
		{
			AudioFinished();
			return;
		}

		while(len > 0) {
			if(audio_buf_index >= audio_buf_size) {
				/* We have already sent all our data; get more */
				audio_size = audio_decode_frame(&pts);
				if(audio_size < 0) {
					/* If error, output silence */
					audio_buf_size = 1024;
					memset(audio_buf, 0, audio_buf_size);
				} else {
					audio_size = synchronize_audio((int16_t *)audio_buf,
						audio_size, pts);
					audio_buf_size = audio_size;
				}
				audio_buf_index = 0;
			}

			len1 = audio_buf_size - audio_buf_index;
			if(len1 > (int)len)
				len1 = len;

			if(len1 > 0)
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

		if(av_sync_type != AV_SYNC_AUDIO_MASTER) {
			double diff, avg_diff;
			int wanted_size, min_size, max_size;

			ref_clock = get_master_clock();
			diff = get_audio_clock() - ref_clock;
			if(diff < AV_NOSYNC_THRESHOLD) {
				// accumulate the diffs
				audio_diff_cum = diff + audio_diff_avg_coef
					* audio_diff_cum;
				if(audio_diff_avg_count < AUDIO_DIFF_AVG_NB) {
					audio_diff_avg_count++;
				} else {
					avg_diff = audio_diff_cum * (1.0 - audio_diff_avg_coef);
					if(fabs(avg_diff) >= audio_diff_threshold) {
						wanted_size = samples_size + ((int)(diff * audio_st->codec->sample_rate) * n);
						min_size = samples_size * ((100 - SAMPLE_CORRECTION_PERCENT_MAX) / 100);
						max_size = samples_size * ((100 + SAMPLE_CORRECTION_PERCENT_MAX) / 100);
						if(wanted_size < min_size) {
							wanted_size = min_size;
						} else if (wanted_size > max_size) {
							wanted_size = max_size;
						}
						if(wanted_size < samples_size) {
							/* remove samples */
							samples_size = wanted_size;
						} else if(wanted_size > samples_size) {
							uint8_t *samples_end, *q;
							int nb;
							/* add samples by copying final sample*/
							nb = (samples_size - wanted_size);
							samples_end = (uint8_t *)samples + samples_size - n;
							q = samples_end + n;
							while(nb > 0) {
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

		for(;;) {
			while(pkt2->size > 0) {
				data_size = sizeof(audio_buf1);
				len1 = FFmpeg::avcodec_decode_audio3(dec, 
					(int16_t *)audio_buf1, &data_size, 
					pkt2);

#ifdef SIMULATE_SLOW_CPU
				Sleep(AUDIO_DECODE_STALL);
#endif

				if(len1 < 0) {
					/* if error, skip frame */
					pkt2->size = 0;
					// try to avoid inifinite loop if something goes wrong
					if (++failed_frames > 512)
						return -1;
					break;
				}

				pkt2->data += len1;
				pkt2->size -= len1;
				if(data_size <= 0) {
					/* No data yet, get more frames */
					continue;
				}

				if (dec->sample_fmt != audio_src_fmt) {
					if (reformat_ctx)
						FFmpeg::av_audio_convert_free(reformat_ctx);

					reformat_ctx = FFmpeg::av_audio_convert_alloc(AV_SAMPLE_FMT_S16, 1, dec->sample_fmt, 1, NULL, 0);

					if (!reformat_ctx) {
						Warning(("Cannot convert %s sample format to %s sample format\n",
							FFmpeg::av_get_sample_fmt_name(dec->sample_fmt),
							FFmpeg::av_get_sample_fmt_name(AV_SAMPLE_FMT_S16)));
						break;
					}

					audio_src_fmt = dec->sample_fmt;
				}

				if (reformat_ctx) {
					const void *ibuf[6]= { audio_buf1 };
					void *obuf[6] = { audio_buf2 };
					int istride[6] = { FFmpeg::av_get_bits_per_sample_fmt(dec->sample_fmt)/8 };
					int ostride[6] = { 2 };
					int len = data_size/istride[0];

					if (FFmpeg::av_audio_convert(reformat_ctx, obuf, ostride, ibuf, istride, len)<0) {
						Warning(("av_audio_convert() failed\n"));
						break;
					}

					audio_buf = audio_buf2;
					/* FIXME: existing code assume that data_size equals framesize*channels*2 remove this legacy cruft */
					data_size = len*2;
				} else {
					audio_buf = audio_buf1;
				}

				pts = audio_clock;
				*pts_ptr = pts;
				n = 2 * dec->channels;
				audio_clock += (double)data_size / (double)(n * dec->sample_rate);

				/* We have data, return it and come back for more later */
				return data_size;
			}

			/* free the current packet */
			if(pkt->data)
				FFmpeg::av_free_packet(pkt);

			if(quit) {
				return -1;
			}

			/* read next packet */
			if(audioq.Get(pkt, 1) < 0) {
				return -1;
			}

			pkt2->data = pkt->data;
			pkt2->size = pkt->size;

			/* if update, update the audio clock w/pts */
			if(pkt->pts != AV_NOPTS_VALUE) {
				audio_clock = av_q2d(audio_st->time_base)*pkt->pts;
			}
		}
	}
};


///////////////////////////////////////////////////////////////////////////////
//
// DecodeThread
//

class DecodeThread : public cWorkerThread
{
public:
	DecodeThread(VideoState *is) : is(is) {};
protected:
	virtual DWORD ThreadProc()
	{
		AVPacket pkt;
		AVPacket *packet = &pkt;

		FFmpeg::av_init_packet(packet);

		int eof = 0;

		// main decode loop
		for(;;) {
			if(is->quit) {
				break;
			}

			// if the queue are full, no need to read more
			if ((is->audioq.size > MIN_AUDIOQ_SIZE || is->audioStream < 0)
				&& (is->videoq.nb_packets > MIN_FRAMES || is->videoStream < 0)
				/*&& (is->subtitleq.nb_packets > MIN_FRAMES || is->subtitle_stream < 0)*/) {
					Sleep(10);
					continue;
			}

			if(eof) {
				// wait for queues to run empty
				Sleep(10);
				if(is->audioq.size + is->videoq.size /*+ is->subtitleq.size*/ == 0) {
					break;
				}
				continue;
			}

			if(FFmpeg::av_read_frame(is->pFormatCtx, packet) < 0) {
				eof = 1;
				// signal queues that nothing more is coming
				is->videoq.Depleted();
				is->audioq.Depleted();
				continue;
			}
			// Is this a packet from the video stream?
			if(packet->stream_index == is->videoStream) {
				is->videoq.Put(packet);
			} else if(packet->stream_index == is->audioStream) {
				is->audioq.Put(packet);
			} else {
				FFmpeg::av_free_packet(packet);
			}			
		}

		is->quit = 1;
		is->DecodeFinished();

		return 0;
	}



private:
	VideoState *is;
};

#pragma pack()


static uint64_t global_video_pkt_pts = AV_NOPTS_VALUE;

/*int our_get_buffer(struct AVCodecContext *c, AVFrame *pic)
{
	int ret = FFmpeg::avcodec_default_get_buffer(c, pic);
	uint64_t *pts = (uint64_t*)FFmpeg::av_malloc(sizeof(uint64_t));
	*pts = global_video_pkt_pts;
	pic->opaque = pts;
	return ret;
}

int our_reget_buffer(AVCodecContext *codec, AVFrame *pic)
{
	if (pic->data[0] == NULL) {
		pic->buffer_hints |= FF_BUFFER_HINTS_READABLE;
		return codec->get_buffer(codec, pic);
	}

	return 0;
}

void our_release_buffer(struct AVCodecContext *c, AVFrame *pic) 
{
	if(pic) FFmpeg::av_freep(&pic->opaque);
	FFmpeg::avcodec_default_release_buffer(c, pic);
}*/


///////////////////////////////////////////////////////////////////////////////
//
// VideoThread
//

class VideoThread : public cWorkerThread
{
public:
	VideoThread(VideoState *is) : is(is)
	{
	}
protected:	
    virtual DWORD ThreadProc()
	{
		AVPacket pkt1;
		AVPacket *packet = &pkt1;
		int len1, frameFinished;
		AVFrame *pFrame;
		double pts;
		int64_t pts_int;

		pFrame = FFmpeg::avcodec_alloc_frame();

		for(;;) {
			if(is->videoq.Get(packet, 1) < 0) {
				// means we quit getting packets
				break;
			}

			pts_int = 0;

			// Save global pts to be stored in pFrame
			global_video_pkt_pts = packet->pts;
			// Decode video frame
			len1 = FFmpeg::avcodec_decode_video2(is->video_st->codec, pFrame, &frameFinished, packet);

#ifdef SIMULATE_SLOW_CPU
			Sleep(VIDEO_DECODE_STALL);
#endif

			// Did we get a video frame?
			if(frameFinished) {
				pts_int = pFrame->best_effort_timestamp;

				if (pts_int == AV_NOPTS_VALUE) {
					pts_int = 0;
				}
				pts = (double)pts_int * av_q2d(is->video_st->time_base);

				pts = synchronize_video(pFrame, pts);

				is->skip_frames_index += 1;
				if(is->skip_frames_index >= is->skip_frames) {
					is->skip_frames_index -= FFMAX(is->skip_frames, 1.0);

					if(queue_picture(pFrame, pts, packet->pos) < 0) {
						FFmpeg::av_free_packet(packet);
						break;
					}
				}
#ifdef _DEBUG
				else
				{
					Warning(("ffmpeg: dropped decoded frame sfi:%g sf:%g pts:%g pktpts:%g\n", is->skip_frames_index, is->skip_frames, (double)pts, packet->pts));
				}
#endif
			}
			FFmpeg::av_free_packet(packet);
		}
		FFmpeg::av_free(pFrame);

		is->VideoFinished();

		return 0;
	}
private:	
	double synchronize_video(AVFrame *src_frame, double pts)
	{
		double frame_delay;

		if(pts != 0) {
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
		is->pictq_mutex.Wait();
		if(is->pictq_size>=VIDEO_PICTURE_QUEUE_SIZE && !is->refresh)
			is->skip_frames= FFMAX(1.0 - FRAME_SKIP_FACTOR, is->skip_frames * (1.0-FRAME_SKIP_FACTOR));
		while(is->pictq_size >= VIDEO_PICTURE_QUEUE_SIZE &&	!is->quit) 
		{
			SDL_CondWait(is->pictq_cond, is->pictq_mutex);
		}
		is->pictq_mutex.Release();

		if(is->quit)
			return -1;

		// windex is set to 0 initially
		vp = &is->pictq[is->pictq_windex];

		ILGVideoDecoderHost::sLockResult lock;
		if ( !is->pOuter->m_pHostIface->LockBuffer(vp->bmp, lock) )
			return -1;

		ILGVideoDecoderHost::sFrameFormat fmt;
		is->pOuter->m_pHostIface->GetFrameFormat(fmt);

		uint8 *data[] = {(uint8*)lock.buffer, NULL, NULL};
		int stride[] = {lock.pitch, 0, 0};

		is->img_convert_ctx = FFmpeg::sws_getCachedContext(is->img_convert_ctx,
			is->video_st->codec->width, is->video_st->codec->height, is->video_st->codec->pix_fmt,
			fmt.width, fmt.height, is->pict_pix_fmt,
			sws_flags, NULL, NULL, NULL);

		FFmpeg::sws_scale(is->img_convert_ctx, pFrame->data,
			pFrame->linesize, 0,
			is->video_st->codec->height, data, stride);

        //BBi
        subtitles.processPicture (pts, is->subtitles, fmt, lock);
        //BBi

		is->pOuter->m_pHostIface->UnlockBuffer(vp->bmp);

		vp->pts = pts;
		vp->pos = pos;

		// now we inform our display thread that we have a pic ready
		if(++is->pictq_windex == VIDEO_PICTURE_QUEUE_SIZE) 
		{
			is->pictq_windex = 0;
		}
		is->pictq_mutex.Wait();
		vp->target_clock = is->compute_target_time(vp->pts);
		is->pictq_size++;
		is->pictq_mutex.Release();

		return 0;
	}

	VideoState *is;

    //BBi
    Subtitles subtitles;
    //BBi
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

    global_video_pkt_pts = AV_NOPTS_VALUE;
    
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
	if(audio_st)
		audio_diff_threshold = 2.0 * AUDIO_BUFFER_SIZE / (double)audio_st->codec->sample_rate;
	else
		audio_diff_threshold = 0;

    memset(&audio_pkt, 0, sizeof(audio_pkt));
    memset(&audio_pkt2, 0, sizeof(audio_pkt2));

	const int64_t curtime = FFmpeg::av_gettime();

    frame_timer = (double)curtime / 1000000.0;
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
	if ( !video_tid->Create() )
	{
		AssertMsg(FALSE, "VideoThread::Create");
	}

	schedule_refresh(40);

	Assert_(parse_tid == NULL);
	parse_tid = new DecodeThread(this);
	if ( !parse_tid->Create() )
	{
		AssertMsg(FALSE, "DecodeThread::Create");
	}

	return (parse_tid != 0);
}

int VideoState::stream_component_open(int stream_index) 
{
	AVCodecContext *codecCtx;
	AVCodec *codec;

	if(stream_index < 0 || stream_index >= (int)pFormatCtx->nb_streams) {
		return -1;
	}

	// Get a pointer to the codec context for the video stream
	codecCtx = pFormatCtx->streams[stream_index]->codec;

	/* prepare audio output */
	if (codecCtx->codec_type == AVMEDIA_TYPE_AUDIO) {
		if (codecCtx->channels > 0) {
			codecCtx->request_channels = FFMIN(2, codecCtx->channels);
		} else {
			codecCtx->request_channels = 2;
		}
	}

	codec = FFmpeg::avcodec_find_decoder(codecCtx->codec_id);

#ifdef _DEBUG
    codecCtx->debug = FF_DEBUG_BUGS|/*FF_DEBUG_VIS_MB_TYPE|*/FF_DEBUG_ER|/*FF_DEBUG_SKIP|FF_DEBUG_PICT_INFO|*/FF_DEBUG_PTS;
#endif

    /*codecCtx->debug_mv = 0;
    codecCtx->debug = 0;
    codecCtx->workaround_bugs = 1;
    //codecCtx->flags |= CODEC_FLAG_EMU_EDGE;
    codecCtx->idct_algo = FF_IDCT_AUTO;
    //if (fast) codecCtx->flags2 |= CODEC_FLAG2_FAST;
    codecCtx->skip_frame = AVDISCARD_DEFAULT;
    codecCtx->skip_idct = AVDISCARD_DEFAULT;
    codecCtx->skip_loop_filter = AVDISCARD_DEFAULT;
    codecCtx->error_recognition = FF_ER_CAREFUL;
    codecCtx->error_concealment = 3;
    codecCtx->thread_count = 1;*/

	if(!codec || (FFmpeg::avcodec_open(codecCtx, codec) < 0)) {
		AssertMsg(FALSE, "Unsupported codec!\n");
		return -1;
	}

	pFormatCtx->streams[stream_index]->discard = AVDISCARD_DEFAULT;

	switch(codecCtx->codec_type)
	{
	case AVMEDIA_TYPE_AUDIO:
		{
			// we want 16-bit samples out from the movie (they'll be converted if necessary)
			audio_src_fmt = AV_SAMPLE_FMT_S16;

			if ( !pOuter->m_pHostIface->CreateAudioBuffer(codecCtx->sample_rate, codecCtx->channels, AUDIO_BUFFER_SIZE) )
				break;

			audioStream = stream_index;
			audio_st = pFormatCtx->streams[stream_index];

			memset(&audio_pkt, 0, sizeof(audio_pkt));
		}
		break;

	case AVMEDIA_TYPE_VIDEO:
		videoStream = stream_index;
		video_st = pFormatCtx->streams[stream_index];
        
        /*codecCtx->get_buffer = our_get_buffer;
        codecCtx->release_buffer = our_release_buffer;
		codecCtx->reget_buffer = our_reget_buffer;
		//codecCtx->thread_safe_callbacks = 1;*/
		break;

    /*case AVMEDIA_TYPE_SUBTITLE:
        break;*/

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

	//CancelWaitableTimer(timer);

	if(video_tid)
		video_tid->WaitForClose();
	delete video_tid;
	video_tid = 0;

	if(parse_tid)
		parse_tid->WaitForClose();
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

	if (is->video_st && is->pictq_size)
	{
		// TODO: should we check if it's still to early to display frame and return FALSE?
		return TRUE;
	}

	return FALSE;
}

STDMETHODIMP_(void) cLGVideoDecoder::RequestVideoFrame()
{
	if (is && is->refresh)
	{
		is->video_refresh_timer();
		is->refresh = 0;
	}
}

STDMETHODIMP_(void) cLGVideoDecoder::RequestAudio(unsigned int len)
{
	if (is)
		is->audio_request(len);
}

//

void cLGVideoDecoder::Stop()
{
	if (is)
	{
		is->Stop();
		is->Close();

		delete is;
		is = NULL;
	}

	FFmpeg::Shutdown();
}

BOOL cLGVideoDecoder::Init(const char *filename)
{
	char buf[16] = {0,};

	sws_flags = SWS_BICUBIC;
	if ( m_pHostIface->GetConfigValue("movie_sw_scale_quality", buf, sizeof(buf)) )
	{
		sws_flags = atoi(buf);

		if (sws_flags < 0)
			sws_flags = 0;
		else if (sws_flags > 6)
			sws_flags = 6;

		switch (sws_flags)
		{
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

	if ( !FFmpeg::Init(this) )
		return FALSE;

	FFmpeg::avcodec_init();
	FFmpeg::av_register_all();

	is = new VideoState(this);

	//

	ILGVideoDecoderHost::sFrameFormat fmt;
	m_pHostIface->GetFrameFormat(fmt);

	PixelFormat pixformat = PIX_FMT_NONE;
	
	if(fmt.bpp == 32)
	{
		if(fmt.gmask == 0xFF0000) // && bmask.alpha == 0xFF
		{
			if(fmt.rmask == 0xFF00  && fmt.bmask == 0xFF000000)
				pixformat = PIX_FMT_ARGB;
			else if(fmt.rmask == 0xFF000000 && fmt.bmask == 0xFF00)
				pixformat = PIX_FMT_ABGR;
		}
		else if(fmt.gmask == 0xFF00) // && bmask.alpha == 0xFF000000
		{
			if(fmt.rmask == 0xFF && fmt.bmask == 0xFF0000 )
				pixformat = PIX_FMT_RGBA;
			else if(fmt.rmask == 0xFF0000 && fmt.bmask == 0xFF)
				pixformat = PIX_FMT_BGRA;
		}
	}
	else if(fmt.bpp == 16)
	{
		if(fmt.gmask == 0x7E0)
		{
			if(fmt.bmask == 0x1F)
				pixformat = PIX_FMT_RGB565LE;
			else
				pixformat = PIX_FMT_BGR565LE;
		}
	}

	if (pixformat == PIX_FMT_NONE)
	{
		// failed to find suitable pixel format (probably in 8-bit mode)
		Assert_(pixformat != PIX_FMT_NONE);
		Stop();
		return FALSE;
	}

	if (!is->Open(filename, fmt.width, fmt.height, pixformat))
	{
		Stop();
		return FALSE;
	}

	return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
//
// DLL interface
//

#ifdef _MSC_VER
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

extern "C" DLLEXPORT ILGVideoDecoder* CreateLGVideoDecoder(ILGVideoDecoderHost *pHostIface, const char *filename)
{
	if (!pHostIface || !filename)
		return NULL;

	cLGVideoDecoder *p = new cLGVideoDecoder(pHostIface);

	if ( !p->Init(filename) )
	{
		delete p;
		return NULL;
	}

	return p;
}
