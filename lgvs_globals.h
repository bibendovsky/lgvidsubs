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


#ifndef LGVS_GLOBALS_H
#define LGVS_GLOBALS_H


#include "lgvs_subtitle.h"


namespace lgvs {


class Globals {
public:
    static const float DEFAULT_SPACE_AFTER_PCT;
    static const float MIN_SPACE_AFTER_PCT;
    static const float MAX_SPACE_AFTER_PCT;

    static const float DEFAULT_SPACE_AFTER_PIX;
    static const float MIN_SPACE_AFTER_PIX;
    static const float MAX_SPACE_AFTER_PIX;

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


    static float space_after;
    static bool space_after_in_percents;

    static float font_size;
    static bool font_size_in_percents;
    static float font_weight;
    static unsigned font_color;
    static std::wstring font_family;

    static unsigned shadow_color;
    static float shadow_offset_x;
    static bool shadow_offset_x_in_percents;
    static float shadow_offset_y;
    static bool shadow_offset_y_in_percents;

    static bool enable_subs;
    static int sub_index;
    static SubtitleList subs;


private:
    Globals();
    Globals(const Globals& that);
    ~Globals();
    Globals& operator=(const Globals& that);
}; // class Globals


} // namespace lgvs


#endif // LGVS_GLOBALS_H
