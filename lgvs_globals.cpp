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


#include "lgvs_globals.h"


namespace lgvs {


const float Globals::DEFAULT_SPACE_AFTER_PCT = DEFAULT_FONT_SIZE_PCT * 0.5F;
const float Globals::MIN_SPACE_AFTER_PCT     = 0.0F;
const float Globals::MAX_SPACE_AFTER_PCT     = MAX_FONT_SIZE_PCT;

const float Globals::DEFAULT_SPACE_AFTER_PIX = 18.0F;
const float Globals::MIN_SPACE_AFTER_PIX     = 0.0F;
const float Globals::MAX_SPACE_AFTER_PIX     = MAX_FONT_SIZE_PCT;

const float Globals::DEFAULT_FONT_SIZE_PCT = 7.0F;
const float Globals::MIN_FONT_SIZE_PCT     = 1.5F;
const float Globals::MAX_FONT_SIZE_PCT     = 13.0F;

const float Globals::DEFAULT_FONT_SIZE_PIX = 18.0F;
const float Globals::MIN_FONT_SIZE_PIX     = 8.0F;
const float Globals::MAX_FONT_SIZE_PIX     = 64.0F;

const float Globals::DEFAULT_FONT_WEIGHT = 0.0F;
const float Globals::MIN_FONT_WEIGHT     = 0.0F;
const float Globals::MAX_FONT_WEIGHT     = 1000.0F;

const float Globals::MIN_COLOR_COMPONENT = 0.0F;
const float Globals::MAX_COLOR_COMPONENT = 1.0F;

const unsigned Globals::DEFAULT_FONT_COLOR   = 0xFFFFFFFF;
const unsigned Globals::DEFAULT_SHADOW_COLOR = 0xFF000000;

const float Globals::DEFAULT_SHADOW_OFFSET_PCT = 0.42F;
const float Globals::MIN_SHADOW_OFFSET_PCT     = -1.45F;
const float Globals::MAX_SHADOW_OFFSET_PCT     = 1.45F;

const float Globals::DEFAULT_SHADOW_OFFSET_PIX = 2.0F;
const float Globals::MIN_SHADOW_OFFSET_PIX     = -7.0F;
const float Globals::MAX_SHADOW_OFFSET_PIX     = 7.0F;


float Globals::space_after;
bool Globals::space_after_in_percents;

float Globals::font_size;
bool Globals::font_size_in_percents;
float Globals::font_weight;
unsigned Globals::font_color;
std::wstring Globals::font_family;

unsigned Globals::shadow_color;
float Globals::shadow_offset_x;
bool Globals::shadow_offset_x_in_percents;
float Globals::shadow_offset_y;
bool Globals::shadow_offset_y_in_percents;

bool Globals::enable_subs;
int Globals::sub_index;
SubtitleList Globals::subs;


} // namespace lgvs
