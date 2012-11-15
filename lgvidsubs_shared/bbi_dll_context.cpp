#include "bbi_dll_context.h"
#include "bbi_subtitle.h"


namespace bbi {


    const float DllContext::DEFAULT_FONT_SIZE_PCT = 7.0F;
    const float DllContext::MIN_FONT_SIZE_PCT     = 1.5F;
    const float DllContext::MAX_FONT_SIZE_PCT     = 13.0F;

    const float DllContext::DEFAULT_FONT_SIZE_PIX = 18.0F;
    const float DllContext::MIN_FONT_SIZE_PIX     = 8.0F;
    const float DllContext::MAX_FONT_SIZE_PIX     = 64.0F;

    const float DllContext::DEFAULT_FONT_WEIGHT = 0.0F;
    const float DllContext::MIN_FONT_WEIGHT     = 0.0F;
    const float DllContext::MAX_FONT_WEIGHT     = 1000.0F;

    const float DllContext::MIN_COLOR_COMPONENT = 0.0F;
    const float DllContext::MAX_COLOR_COMPONENT = 1.0F;

    const unsigned DllContext::DEFAULT_FONT_COLOR   = 0xFFFFFFFF;
    const unsigned DllContext::DEFAULT_SHADOW_COLOR = 0xFF000000;

    const float DllContext::DEFAULT_SHADOW_OFFSET_PCT = 0.42F;
    const float DllContext::MIN_SHADOW_OFFSET_PCT     = -1.45F;
    const float DllContext::MAX_SHADOW_OFFSET_PCT     = 1.45F;

    const float DllContext::DEFAULT_SHADOW_OFFSET_PIX = 2.0F;
    const float DllContext::MIN_SHADOW_OFFSET_PIX     = -7.0F;
    const float DllContext::MAX_SHADOW_OFFSET_PIX     = 7.0F;

    const float DllContext::DEFAULT_SPACE_AFTER_PCT = DllContext::DEFAULT_FONT_SIZE_PCT * 0.5F;
    const float DllContext::MIN_SPACE_AFTER_PCT     = 0.0F;
    const float DllContext::MAX_SPACE_AFTER_PCT     = DllContext::MAX_FONT_SIZE_PCT;

    const float DllContext::DEFAULT_SPACE_AFTER_PIX = 18.0F;
    const float DllContext::MIN_SPACE_AFTER_PIX     = 0.0F;
    const float DllContext::MAX_SPACE_AFTER_PIX     = DllContext::MAX_FONT_SIZE_PCT;


    DllContext::DllContext () :
        showSubs (false),
        subIndex (-1),
        lineCount (0)
    {
    }


    void DllContext::importSubtile (
        const bbi::Subtitle* subtitle)
    {
        if (subtitle != 0) {
            lineCount = subtitle->lines.size ();

            for (int i = 0; i < lineCount; ++i) {
                const bbi::WString& line = subtitle->lines[i];

                int length = std::min (MAX_LINE_LENGTH, static_cast<int> (line.size ()));

                bbi::WString::traits_type::copy (
                    lines[i], line.c_str (), length);

                lines[i][length] = '\0';
                linesLengths[i] = length;
            }
        } else {
            lineCount = 0;
        }
    }


}; // namespace bbi