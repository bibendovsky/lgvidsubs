#ifndef BBI_DLL_CONTEXT_H
#define BBI_DLL_CONTEXT_H


namespace bbi {


class Subtitle;


class DllContext {
public:
    static const float DEFAULT_FONT_SIZE_PCT;
    static const float MIN_FONT_SIZE_PCT;
    static const float MAX_FONT_SIZE_PCT;

    static const float DEFAULT_FONT_SIZE_PIX;
    static const float MIN_FONT_SIZE_PIX;
    static const float MAX_FONT_SIZE_PIX;

    static const float DEFAULT_FONT_WEIGHT;
    static const float MIN_FONT_WEIGHT;
    static const float MAX_FONT_WEIGHT;

    static const float MIN_COLOR_COMPONENT;
    static const float MAX_COLOR_COMPONENT;

    static const unsigned DEFAULT_FONT_COLOR;
    static const unsigned DEFAULT_SHADOW_COLOR;

    static const float DEFAULT_SHADOW_OFFSET_PCT;
    static const float MIN_SHADOW_OFFSET_PCT;
    static const float MAX_SHADOW_OFFSET_PCT;

    static const float DEFAULT_SHADOW_OFFSET_PIX;
    static const float MIN_SHADOW_OFFSET_PIX;
    static const float MAX_SHADOW_OFFSET_PIX;

    static const float DEFAULT_SPACE_AFTER_PCT;
    static const float MIN_SPACE_AFTER_PCT;
    static const float MAX_SPACE_AFTER_PCT;

    static const float DEFAULT_SPACE_AFTER_PIX;
    static const float MIN_SPACE_AFTER_PIX;
    static const float MAX_SPACE_AFTER_PIX;

    static const int MAX_LINE_COUNT = 8;
    static const int MAX_LINE_SIZE = 64;
    static const int MAX_LINE_LENGTH = MAX_LINE_SIZE - 1;


    bool show_subs;
    int sub_index;

    int line_count;
    wchar_t lines[MAX_LINE_COUNT][MAX_LINE_SIZE];
    int lines_lengths[MAX_LINE_COUNT];

    float font_size;
    bool font_size_in_percents;
    float font_weight;
    unsigned font_color;
    wchar_t font_family[MAX_LINE_SIZE];

    unsigned shadow_color;
    float shadow_offset_x;
    bool shadow_offset_x_in_percents;
    float shadow_offset_y;
    bool shadow_offset_y_in_percents;

    float space_after;
    bool space_after_in_percents;


    DllContext ();


    void import_subtile (const bbi::Subtitle* subtitle);
}; // class DllContext


}; // namespace bbi


#endif // BBI_DLL_CONTEXT_H
