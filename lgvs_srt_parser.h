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


#ifndef LGVS_SRT_PARSER_H
#define LGVS_SRT_PARSER_H


//
// Parser for .srt subtitles.
//


#include <iostream>

#include "lgvs_subtitle.h"


namespace lgvs {


class SrtParser {
public:
    static SubtitleList parse (std::istream& stream);

private:
    SrtParser ();
    SrtParser (const SrtParser& that);
    ~SrtParser ();
    SrtParser& operator = (const SrtParser& that);

    // Reads a one line from a stream.
    static bool read_line (std::istream& stream, std::wstring& line);
}; // class SrtParser


} // namespace lgvs


#endif // LGVS_SRT_PARSER_H
