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


#ifndef LGVS_DLL_CONTEXT_H
#define LGVS_DLL_CONTEXT_H


#include "lgvs_subtitle.h"


namespace lgvs {


class GlobalInfo {
public:
    static const float DEFAULT_SPACE_AFTER_PCT;
    static const float MIN_SPACE_AFTER_PCT;
    static const float MAX_SPACE_AFTER_PCT;

    static const float DEFAULT_SPACE_AFTER_PIX;
    static const float MIN_SPACE_AFTER_PIX;
    static const float MAX_SPACE_AFTER_PIX;

    float space_after;
    bool space_after_in_percents;


    GlobalInfo ();
    GlobalInfo (const GlobalInfo& that);
    ~GlobalInfo ();
    GlobalInfo& operator = (const GlobalInfo& that);

    void reset ();
}; // class GlobalInfo


class GlobalFontInfo {
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


    float font_size;
    bool font_size_in_percents;
    float font_weight;
    unsigned font_color;
    std::wstring font_family;

    unsigned shadow_color;
    float shadow_offset_x;
    bool shadow_offset_x_in_percents;
    float shadow_offset_y;
    bool shadow_offset_y_in_percents;


    GlobalFontInfo ();
    GlobalFontInfo (const GlobalFontInfo& that);
    ~GlobalFontInfo ();
    GlobalFontInfo& operator = (const GlobalFontInfo& that);

    void reset ();
}; // class GlobalFontInfo


typedef void (*FP_LGS_SET_GLOBAL_INFO) (const GlobalInfo& font_info);
typedef void (*FP_LGS_SET_GLOBAL_FONT_INFO) (const GlobalFontInfo& font_info);
typedef void (*FP_LGS_SET_SUBTITLES) (const lgvs::SubtitleList& subs);
typedef void (*FP_LGS_SHOW_SUBTITLE) (int index);
typedef void (*FP_LGS_ENABLE_SUBTITLES) (bool value);


} // namespace lgvs


#endif // LGVS_DLL_CONTEXT_H
