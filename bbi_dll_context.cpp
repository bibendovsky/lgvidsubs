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
    show_subs (false),
    sub_index (-1),
    line_count (0)
{
}

void DllContext::import_subtile (const bbi::Subtitle* subtitle)
{
    if (subtitle == NULL) {
        line_count = 0;
        return;
    }

    line_count = subtitle->lines.size ();

    for (int i = 0; i < line_count; ++i) {
        const bbi::WString& line = subtitle->lines[i];

        int length = std::min (static_cast<int> (MAX_LINE_LENGTH),
            static_cast<int> (line.size ()));

        bbi::WString::traits_type::copy (
            lines[i], line.c_str (), length);

        lines[i][length] = L'\0';
        lines_lengths[i] = length;
    }
}


}; // namespace bbi
