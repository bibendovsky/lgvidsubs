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


#include "lgvs_subtitle.h"


namespace lgvs {


Subtitle::Subtitle ()
{
    reset ();
}

Subtitle::Subtitle (const Subtitle& that) :
    index (that.index),
    original_index (that.original_index),
    time_begin (that.time_begin),
    time_end (that.time_end),
    lines (that.lines)
{
}

Subtitle::~Subtitle ()
{
}

Subtitle& Subtitle::operator = (const Subtitle& that)
{
    if (&that != this) {
        index = that.index;
        original_index = that.original_index;
        time_begin = that.time_begin;
        time_end = that.time_end;
        lines = that.lines;
    }

    return *this;
}

void Subtitle::reset ()
{
    index = 0;
    original_index = 0;
    time_begin = 0;
    time_end = 0;
    lines.clear ();
}


} // namespace lgvs
