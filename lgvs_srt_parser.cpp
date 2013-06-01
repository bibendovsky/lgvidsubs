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


#include "lgvs_srt_parser.h"

#include <algorithm>
#include <sstream>

#include "lgvs_string_helper.h"


namespace lgvs {


// (static)
SubtitleList SrtParser::parse (std::istream& stream)
{
    enum State {
        // Search for a non-blank line
        ST_SKIP_BLANK,

        // Parse an index
        ST_PARSE_INDEX,

        // Read a line with timestamps
        ST_READ_TIMESTAMPS,

        // Parse timestamps
        ST_PARSE_TIMESTAMPS,

        // Read a line with text
        ST_READ_TEXT,

        // Parse a text
        ST_PARSE_TEXT,

        // Error happens
        ST_ERROR,

        // Finish parsing
        ST_QUIT,
    }; // enum State


    std::wstring line;
    Subtitle subbuffer;
    SubtitleList subtitles;

    // Subtitle's lines counter
    int subline_count;

    for (State state = ST_SKIP_BLANK; state != ST_QUIT; ) {
        switch (state) {
        case ST_SKIP_BLANK:
            if (read_line (stream, line)) {
                if (!line.empty ()) {
                    subbuffer.reset ();
                    state = ST_PARSE_INDEX;
                }
            } else
                state = ST_QUIT;
            break;

        case ST_PARSE_INDEX:
            {
                // Format of a line:
                // index

                int index;
                std::wistringstream line_stream (line);
                line_stream >> index;

                if (!line_stream.fail ()) {
                    subbuffer.original_index = index;
                    state = ST_READ_TIMESTAMPS;
                } else
                    state = ST_ERROR;
            }
            break;

        case ST_READ_TIMESTAMPS:
            if (read_line (stream, line))
                state = ST_PARSE_TIMESTAMPS;
            else
                state = ST_ERROR;
            break;

        case ST_PARSE_TIMESTAMPS:
            {
                // Format of a line:
                // hh1:mm1:ss1,mss1 --> hh2:mm2:ss2,mss2
                // where hh - hours, mm - minutes, ss - seconds, mss - milliseconds

                int hh1;
                int mm1;
                int ss1;
                int mss1;
                int hh2;
                int mm2;
                int ss2;
                int mss2;
                wchar_t sign1_1;
                wchar_t sign1_2;
                wchar_t sign1_3;
                wchar_t sign2_1;
                wchar_t sign2_2;
                wchar_t sign2_3;
                wchar_t dash1;
                wchar_t dash2;
                wchar_t angle;
                std::wistringstream line_stream (line);

                line_stream >>
                    hh1 >> sign1_1 >> mm1 >> sign1_2 >> ss1 >> sign1_3 >> mss1 >>
                    dash1 >> dash2 >> angle >>
                    hh2 >> sign2_1 >> mm2 >> sign2_2 >> ss2 >> sign2_3 >> mss2;

                if (!line_stream.fail ()) {
                    int msecs1 = (hh1 * 3600000) + (mm1 * 60000) + (ss1 * 1000) + mss1;
                    int msecs2 = (hh2 * 3600000) + (mm2 * 60000) + (ss2 * 1000) + mss2;

                    subbuffer.time_begin = msecs1;
                    subbuffer.time_end = msecs2;
                    subline_count = 0;

                    state = ST_READ_TEXT;
                } else
                    state = ST_ERROR;
            }
            break;

        case ST_READ_TEXT:
            // Format of a line:
            // <text>
            // or
            // \n

            if (read_line (stream, line) &&
                !lgvs::StringHelper::is_empty_or_white_space (line))
            {
                ++subline_count;
                state = ST_PARSE_TEXT;
            } else {
                if (subline_count > 0) {
                    subline_count = 0;
                    subtitles.push_back (subbuffer);
                    state = ST_SKIP_BLANK;
                } else
                    state = ST_ERROR;
            }
            break;

        case ST_PARSE_TEXT:
            if (line.compare (L"\\n") != 0)
                subbuffer.lines.push_back (line);
            else
                subbuffer.lines.push_back (std::wstring ());

            state = ST_READ_TEXT;
            break;

        case ST_ERROR:
            state = ST_QUIT;
            break;
        }

        if (stream.eof ())
            state = ST_QUIT;
    }


    class SortPred {
    public:
        static bool compare (const Subtitle& a, const Subtitle& b)
        {
            return a.original_index < b.original_index;
        }
    };

    std::sort (subtitles.begin (), subtitles.end (), SortPred::compare);

    for (lgvs::SubtitleList::size_type i = 0; i < subtitles.size (); ++i)
        subtitles[i].index = i;

    return subtitles;
}


// (static)
bool SrtParser::read_line (std::istream& stream, std::wstring& line)
{
    std::string buffer;

    std::getline (stream, buffer);

    if (!stream.fail ()) {
        line = StringHelper::to_wstring (buffer);
        return true;
    }

    return false;
}


} // namespace lgvs
