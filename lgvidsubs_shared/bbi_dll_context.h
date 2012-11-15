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


        bool showSubs;
        int subIndex;

        int lineCount;
        wchar_t lines[MAX_LINE_COUNT][MAX_LINE_SIZE];
        int linesLengths[MAX_LINE_COUNT];

        float fontSize;
        bool fontSizeInPercents;
        float fontWeight;
        unsigned fontColor;
        wchar_t fontFamily[MAX_LINE_SIZE];

        unsigned shadowColor;
        float shadowOffsetX;
        bool shadowOffsetXInPercents;
        float shadowOffsetY;
        bool shadowOffsetYInPercents;

        float spaceAfter;
        bool spaceAfterInPercents;


        DllContext ();


        void importSubtile (
            const bbi::Subtitle* subtitle);
    }; // class DllContext


}; // namespace bbi


#endif // BBI_DLL_CONTEXT_H
