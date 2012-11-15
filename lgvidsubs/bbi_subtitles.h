#ifndef BBI_SUBTITLES_H
#define BBI_SUBTITLES_H


#include <windows.h>

#include "bbi_subtitle.h"
#include "lgviddecoder.h"


class Subtitles {
public:
    Subtitles ();

    ~Subtitles ();

    void processPicture (
        double pts, // frame time (in seconds)
        const bbi::SubtitleList& subtitles,
        const ILGVideoDecoderHost::sFrameFormat& frameFormat,
        const ILGVideoDecoderHost::sLockResult& lock);


private:
    HDC mMemDc; // memory DC
    HBITMAP mDib; // segmented bitmap
    HGDIOBJ mDefaultDib;
    DWORD* mDibBits; // pointer to data of bitmap
    HFONT mFont;
    HGDIOBJ mDefaultFont;
    ILGVideoDecoderHost::sFrameFormat mFrameFormat;
    std::vector<SIZE> mLinesSizes;


    Subtitles (
        const Subtitles& that);

    Subtitles& operator = (
        const Subtitles& that);
}; // class Subtitles


#endif // BBI_SUBTITLES_H
