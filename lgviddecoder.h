/* lgvideodecoder.h is free software. It comes without any warranty,
 * to the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details.
 */

#ifndef __LGVIDDECODER_H
#define __LGVIDDECODER_H

#ifndef BOOL
#define BOOL int
#define FALSE 0
#define TRUE 1
#endif


#pragma pack(4)

// host interface that allows a LGVideoDecoder to access necessary host functions
#undef INTERFACE
#define INTERFACE ILGVideoDecoderHost
DECLARE_INTERFACE(ILGVideoDecoderHost)
{
public:
	enum
	{
		SEEK_Set,
		SEEK_Cur,
		SEEK_End
	};

	struct sFrameFormat
	{
		int width;
		int height;
		int bpp;
		unsigned int rmask;
		unsigned int gmask;
		unsigned int bmask;
		// Cropping params are only for information and for placement of possible subtitles
		// the decoder should not actually crop the image, that is done by the host.
		// Note that cropping may be toggle during playback so subtitles would have to always
		// keep up to date for correct positioning.
		BOOL cropped;
		int croprect[4];
	};

	struct sLockResult
	{
		char *buffer;
		int pitch;
	};

public:
	// get config var value, can be used if decoder has any user configurable settings
	STDMETHOD_(BOOL,GetConfigValue)(const char *name, char *buffer, int len) PURE;

	// output string to log (debug output)
	STDMETHOD_(void,LogPrint)(const char *s) PURE;

	//
	// file I/O
	//

	// open file, returns a file handle, NULL if failed
	STDMETHOD_(void*,FileOpen)(const char *filename) PURE;
	// close file handle
	STDMETHOD_(void,FileClose)(void *handle) PURE;
	// get file size
	STDMETHOD_(size_t,FileSize)(void *handle) PURE;
	// read data from file
	STDMETHOD_(size_t,FileRead)(void *handle, void *buf, size_t count) PURE;
	// move current file read pos
	STDMETHOD_(size_t,FileSeek)(void *handle, long offset, int origin) PURE;

	//
	// audio buffer access
	//

	// create audio buffer for sound playback if video contains audio track (may only be called once per decoder)
	// (audio data is expected to be 16-bit signed)
	STDMETHOD_(BOOL,CreateAudioBuffer)(int nSampleRate, int nChannels, int nBufferSize) PURE;

	// load audio data to sound buffer, may only be called from inside RequestAudio (can be called several times)
	// returns number of bytes actually queued
	STDMETHOD_(int,QueueAudioData)(void *data, int len) PURE;

	//
	// video buffer access
	//

	// get frame buffer format
	STDMETHOD_(void,GetFrameFormat)(sFrameFormat &fmt) PURE;

	// create an image buffer that can be used to store a video frame using LockBuffer/UnlockBuffer
	// the decoder can create several buffers to queue multiple frame internally
	// returns handle or NULL if create failed
	STDMETHOD_(void*,CreateImageBuffer)() PURE;

	// lock/unlock image buffer so a video frame can be copied to it
	STDMETHOD_(BOOL,LockBuffer)(void *handle, sLockResult &lock) PURE;
	STDMETHOD_(void,UnlockBuffer)(void *handle) PURE;

	// present an image buffer to the host's video frame buffer, may only be called from inside RequestVideoFrame
	// EndVideoFrame may stall while waiting for vsync if the frame buffer is the screen
	STDMETHOD_(void,BeginVideoFrame)(void *handle) PURE;
	STDMETHOD_(void,EndVideoFrame)() PURE;
};


// decoder interface
#undef INTERFACE
#define INTERFACE ILGVideoDecoder
DECLARE_INTERFACE(ILGVideoDecoder)
{
public:
	// shut down and delete decoder instance
	STDMETHOD_(void,Destroy)() PURE;

	// start decoding for playback
	STDMETHOD_(BOOL,Start)() PURE;

	// returns FALSE as long as video hasn't finished
	STDMETHOD_(BOOL,IsFinished)() PURE;

	// returns TRUE if a video frame is available
	STDMETHOD_(BOOL,IsVideoFrameAvailable)() PURE;

	// called to request another video frame, data is sent to host with BeginVideoFrame/EndVideoFrame
	STDMETHOD_(void,RequestVideoFrame)() PURE;

	// called to request more audio data, data is sent to host with QueueAudioData
	// 'len' is set to TRUE to notify that audio system has completed playing all audio
	// this function can be called from another thread
	STDMETHOD_(void,RequestAudio)(unsigned int len) PURE;
};


typedef ILGVideoDecoder* (*PCREATELGVIDEODECODER)(ILGVideoDecoderHost *pHostIface, const char *filename);

/*

// video decoder DLL interface, this function is called by dark to create a decoder instance for the supplied movie file

extern "C" __declspec(dllexport) ILGVideoDecoder* CreateLGVideoDecoder(ILGVideoDecoderHost *pHostIface, const char *filename);

*/

#pragma pack()

#endif /* !__LGVIDDECODER_H */
