#include <algorithm>
#include <vector>

#include <tchar.h>

#include "bbi_subtitles.h"


namespace {
    // Predicate for searching subtitle with a specified time (milliseconds).
    class SubtitlePred {
    public:
        SubtitlePred (
            int time) :
                mTime (time)
        {
        }

        bool operator () (
            const bbi::Subtitle& subtitle)
        {
            return (mTime >= subtitle.timeBegin) && (mTime <= subtitle.timeEnd);
        }


    private:
        int mTime;
    }; // class SubtitlePred
} // namespace


Subtitles::Subtitles () :
    mMemDc (::CreateCompatibleDC (0)),
    mDib (0),
    mDefaultDib (0),
    mDibBits (0),
    mFont (0),
    mDefaultFont (0),
    mLinesSizes ()
{
    mLinesSizes.reserve (3);

    std::uninitialized_fill_n (
        reinterpret_cast<char*> (&mFrameFormat), sizeof (mFrameFormat), 0);

    if (mMemDc == 0)
        return;

    LOGFONT lf;
    lf.lfHeight = -18;
    lf.lfWidth = 0;
    lf.lfEscapement = 0;
    lf.lfOrientation = 0;
    lf.lfWeight = FW_NORMAL;
    lf.lfItalic = false;
    lf.lfUnderline = false;
    lf.lfStrikeOut = false;
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfOutPrecision = OUT_TT_PRECIS;
    lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lf.lfQuality = DEFAULT_QUALITY;
    lf.lfQuality = DRAFT_QUALITY;
    lf.lfPitchAndFamily = VARIABLE_PITCH | FF_DONTCARE;
    ::_tcscpy (lf.lfFaceName, TEXT ("Arial\0"));

    mFont = ::CreateFontIndirect (&lf);

    if (mFont == 0)
        return;

    mDefaultFont = ::SelectObject (mMemDc, mFont);

    // Disable background
    ::SetBkMode (mMemDc, TRANSPARENT);

    ::SetTextColor (mMemDc, RGB (0xFF, 0xFF, 0xFF));
}

Subtitles::~Subtitles ()
{
    if (mDefaultDib != 0)
        ::SelectObject (mMemDc, mDefaultDib);

    if (mDib != 0)
        ::DeleteObject (mDib);

    if (mDefaultFont != 0)
        ::SelectObject (mMemDc, mDefaultFont);

    if (mFont != 0)
        ::DeleteObject (mFont);

    if (mDefaultFont != 0)
        ::SelectObject (mMemDc, mDefaultFont);

    if (mFont != 0)
        ::DeleteObject (mFont);

    if (mMemDc != 0)
        ::DeleteDC (mMemDc);
}

void Subtitles::processPicture (
    double pts,
    const bbi::SubtitleList& subtitles,
    const ILGVideoDecoderHost::sFrameFormat& frameFormat,
    const ILGVideoDecoderHost::sLockResult& lock)
{
    if (subtitles.empty ())
        return;

    bool create =
        (mDib == 0) ||
        (frameFormat.width != mFrameFormat.width) ||
        (frameFormat.height != mFrameFormat.height) ||
        (frameFormat.bpp != mFrameFormat.bpp) ||
        (frameFormat.rmask != mFrameFormat.rmask) ||
        (frameFormat.gmask != mFrameFormat.gmask) ||
        (frameFormat.bmask != mFrameFormat.bmask) ||
        (frameFormat.cropped != mFrameFormat.cropped) ||
        (frameFormat.croprect[0] != mFrameFormat.croprect[0]) ||
        (frameFormat.croprect[1] != mFrameFormat.croprect[1]) ||
        (frameFormat.croprect[2] != mFrameFormat.croprect[2]) ||
        (frameFormat.croprect[3] != mFrameFormat.croprect[3]);

    mFrameFormat = frameFormat;

    // alpha mask
    DWORD amask = (frameFormat.rmask | frameFormat.gmask | frameFormat.bmask) ^ 0xFFFFFFFF;

    BITMAPV5HEADER bm5h;
    bm5h.bV5Size = sizeof (bm5h);
    bm5h.bV5Width = frameFormat.width;
    bm5h.bV5Height = -frameFormat.height;
    bm5h.bV5Planes = 1;
    bm5h.bV5BitCount = 32;
    bm5h.bV5Compression = BI_RGB;
    bm5h.bV5Compression = BI_BITFIELDS;
    bm5h.bV5SizeImage = 0;
    bm5h.bV5SizeImage = frameFormat.width * frameFormat.height * 4;
    bm5h.bV5XPelsPerMeter = 0;
    bm5h.bV5YPelsPerMeter = 0;
    bm5h.bV5ClrUsed = 0;
    bm5h.bV5ClrImportant = 0;
    bm5h.bV5RedMask = 0;
    bm5h.bV5GreenMask = 0;
    bm5h.bV5BlueMask =  0;
    bm5h.bV5AlphaMask = 0;
    bm5h.bV5RedMask = frameFormat.rmask;
    bm5h.bV5GreenMask = frameFormat.gmask;
    bm5h.bV5BlueMask =  frameFormat.bmask;
    bm5h.bV5AlphaMask = amask;
    bm5h.bV5CSType = LCS_WINDOWS_COLOR_SPACE;
    bm5h.bV5Endpoints.ciexyzRed.ciexyzX = 0;
    bm5h.bV5Endpoints.ciexyzRed.ciexyzY = 0;
    bm5h.bV5Endpoints.ciexyzRed.ciexyzZ = 0;
    bm5h.bV5Endpoints.ciexyzGreen.ciexyzX = 0;
    bm5h.bV5Endpoints.ciexyzGreen.ciexyzY = 0;
    bm5h.bV5Endpoints.ciexyzGreen.ciexyzZ = 0;
    bm5h.bV5Endpoints.ciexyzBlue.ciexyzX = 0;
    bm5h.bV5Endpoints.ciexyzBlue.ciexyzY = 0;
    bm5h.bV5Endpoints.ciexyzBlue.ciexyzZ = 0;
    bm5h.bV5GammaRed = 0;
    bm5h.bV5GammaGreen = 0;
    bm5h.bV5GammaBlue = 0;
    bm5h.bV5Intent = LCS_GM_ABS_COLORIMETRIC;
    bm5h.bV5ProfileData = 0;
    bm5h.bV5Reserved = 0;

    if (create) {
        if (mDefaultDib != 0) {
            ::SelectObject (mMemDc, mDefaultDib);
            mDefaultDib = 0;
        }

        if (mDib != 0) {
            ::DeleteObject (mDib);
            mDib = 0;
        }

        HDC desktopDc = ::GetDC (0);

        mDib = ::CreateDIBSection (
            desktopDc,
            reinterpret_cast<BITMAPINFO*> (&bm5h),
            DIB_RGB_COLORS,
            reinterpret_cast<void**> (&mDibBits),
            0,
            0);

        ::ReleaseDC (0, desktopDc);

        mDefaultDib = ::SelectObject (mMemDc, mDib);
    }

    if (mDib == 0)
        return;

    // Convert to milliseconds
    int time = static_cast<int> (pts * 1000.0);

    bbi::SubtitleList::const_iterator it = std::find_if (
        subtitles.begin (), subtitles.end (), SubtitlePred (time));

    if (it == subtitles.end ())
        return;


    DWORD* dstBits = reinterpret_cast<DWORD*> (lock.buffer);
    std::uninitialized_copy (dstBits, dstBits + (frameFormat.width * frameFormat.height), mDibBits);

    size_t lineCount = it->lines.size ();
    mLinesSizes.resize (lineCount);

    const bbi::WString space = L" ";

    int height = 0;

    // Calculate subtitles rectangle
    for (size_t i = 0, n = lineCount; i < n; ++i) {
        SIZE& lineSize = mLinesSizes[i];
        const bbi::WString& line = it->lines.at (i).empty () ? space : it->lines.at (i);
        BOOL extResult = ::GetTextExtentPoint32W (mMemDc, line.c_str (), line.size (), &lineSize);
        height += lineSize.cy;
    }

    const int BOTTOM_MARGIN = 4;
    int y = frameFormat.height - height - BOTTOM_MARGIN;

    for (size_t i = 0, n = lineCount; i < n; ++i) {
        const SIZE& lineSize = mLinesSizes.at (i);
        const bbi::WString& line = it->lines.at (i);
        int x = (frameFormat.width - lineSize.cx) / 2;

        BOOL textResult = ::TextOutW (mMemDc, x, y, line.c_str (), line.size ());

        y += lineSize.cy;
    }

    // Wait for GDI to complete
    BOOL flushResult = ::GdiFlush ();

    std::uninitialized_copy (mDibBits, mDibBits + (frameFormat.width * frameFormat.height), dstBits);
}
