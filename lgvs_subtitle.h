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


#ifndef LGVS_SUBTITLE_H
#define LGVS_SUBTITLE_H


#include "lgvs_types.h"


namespace lgvs {


// Container for a one subtitle item.
class Subtitle {
public:
    int index; // a real index of subtitle (after sorting)
    int original_index; // an original index of subtitle (as in source file)
    int time_begin; // time (ms) when to show the subtitle
    int time_end; // time (ms) when to hide the subtitle
    WStringList lines; // text of the subtitle

    Subtitle ();
    Subtitle (const Subtitle& that);
    ~Subtitle ();
    Subtitle& operator = (const Subtitle& that);


    void reset ();
}; // class Subtitle


typedef std::vector<Subtitle> SubtitleList;
typedef SubtitleList::iterator SubtitleListIt;
typedef SubtitleList::const_iterator SubtitleListCIt;


} // namespace lgvs


#endif // LGVS_SUBTITLE_H
