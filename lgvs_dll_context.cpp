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


#include "lgvs_dll_context.h"

#include "lgvs_subtitle.h"


namespace lgvs {


// ============================

const float GlobalInfo::DEFAULT_SPACE_AFTER_PCT =
    GlobalFontInfo::DEFAULT_FONT_SIZE_PCT * 0.5F;

const float GlobalInfo::MIN_SPACE_AFTER_PCT = 0.0F;

const float GlobalInfo::MAX_SPACE_AFTER_PCT =
    GlobalFontInfo::MAX_FONT_SIZE_PCT;

const float GlobalInfo::DEFAULT_SPACE_AFTER_PIX = 18.0F;

const float GlobalInfo::MIN_SPACE_AFTER_PIX = 0.0F;

const float GlobalInfo::MAX_SPACE_AFTER_PIX =
    GlobalFontInfo::MAX_FONT_SIZE_PCT;


GlobalInfo::GlobalInfo ()
{
    reset ();
}

GlobalInfo::GlobalInfo (const GlobalInfo& that) :
    space_after (that.space_after),
    space_after_in_percents (that.space_after_in_percents)
{
}

GlobalInfo::~GlobalInfo ()
{
}

GlobalInfo& GlobalInfo::operator = (const GlobalInfo& that)
{
    if (&that != this) {
        space_after = that.space_after;
        space_after_in_percents = that.space_after_in_percents;
    }

    return *this;
}

void GlobalInfo::reset ()
{
    space_after = DEFAULT_SPACE_AFTER_PCT;
    space_after_in_percents = true;
}


// ============================

const float GlobalFontInfo::DEFAULT_FONT_SIZE_PCT = 7.0F;
const float GlobalFontInfo::MIN_FONT_SIZE_PCT     = 1.5F;
const float GlobalFontInfo::MAX_FONT_SIZE_PCT     = 13.0F;

const float GlobalFontInfo::DEFAULT_FONT_SIZE_PIX = 18.0F;
const float GlobalFontInfo::MIN_FONT_SIZE_PIX     = 8.0F;
const float GlobalFontInfo::MAX_FONT_SIZE_PIX     = 64.0F;

const float GlobalFontInfo::DEFAULT_FONT_WEIGHT = 0.0F;
const float GlobalFontInfo::MIN_FONT_WEIGHT     = 0.0F;
const float GlobalFontInfo::MAX_FONT_WEIGHT     = 1000.0F;

const float GlobalFontInfo::MIN_COLOR_COMPONENT = 0.0F;
const float GlobalFontInfo::MAX_COLOR_COMPONENT = 1.0F;

const unsigned GlobalFontInfo::DEFAULT_FONT_COLOR   = 0xFFFFFFFF;
const unsigned GlobalFontInfo::DEFAULT_SHADOW_COLOR = 0xFF000000;

const float GlobalFontInfo::DEFAULT_SHADOW_OFFSET_PCT = 0.42F;
const float GlobalFontInfo::MIN_SHADOW_OFFSET_PCT     = -1.45F;
const float GlobalFontInfo::MAX_SHADOW_OFFSET_PCT     = 1.45F;

const float GlobalFontInfo::DEFAULT_SHADOW_OFFSET_PIX = 2.0F;
const float GlobalFontInfo::MIN_SHADOW_OFFSET_PIX     = -7.0F;
const float GlobalFontInfo::MAX_SHADOW_OFFSET_PIX     = 7.0F;


GlobalFontInfo::GlobalFontInfo ()
{
    reset ();
}

GlobalFontInfo::GlobalFontInfo (const GlobalFontInfo& that) :
    font_size (that.font_size),
    font_size_in_percents (that.font_size_in_percents),
    font_weight (that.font_weight),
    font_color (that.font_color),
    font_family (),

    shadow_color (that.shadow_color),
    shadow_offset_x (that.shadow_offset_x),
    shadow_offset_x_in_percents (that.shadow_offset_x_in_percents),
    shadow_offset_y (that.shadow_offset_y),
    shadow_offset_y_in_percents (that.shadow_offset_y_in_percents)
{
}

GlobalFontInfo::~GlobalFontInfo ()
{
}

GlobalFontInfo& GlobalFontInfo::operator = (const GlobalFontInfo& that)
{
    if (&that != this) {
        font_size = that.font_size;
        font_size_in_percents = that.font_size_in_percents;
        font_weight = that.font_weight;
        font_color = that.font_color;
        font_family = that.font_family;

        shadow_color = that.shadow_color;
        shadow_offset_x = that.shadow_offset_x;
        shadow_offset_x_in_percents = that.shadow_offset_x_in_percents;
        shadow_offset_y = that.shadow_offset_y;
        shadow_offset_y_in_percents = that.shadow_offset_y_in_percents;
    }

    return *this;
}

void GlobalFontInfo::reset ()
{
    font_size = DEFAULT_FONT_SIZE_PCT;
    font_size_in_percents = true;
    font_weight = DEFAULT_FONT_WEIGHT;
    font_color = DEFAULT_FONT_COLOR;
    font_family.clear ();

    shadow_color = DEFAULT_SHADOW_COLOR;
    shadow_offset_x = DEFAULT_SHADOW_OFFSET_PCT;
    shadow_offset_x_in_percents = true;
    shadow_offset_y = DEFAULT_SHADOW_OFFSET_PCT;
    shadow_offset_y_in_percents = true;
}


} // namespace lgvs
