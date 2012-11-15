#include <sstream>

#include "bbi_srt_parser.h"
#include "bbi_string_helper.h"


namespace bbi {


    // (static)
    SubtitleList SrtParser::parse (
        std::istream& stream)
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


        WString line;
        Subtitle subBuffer;
        SubtitleList subtitles;

        // Subtitle's lines counter
        int subLineCount;

        for (State state = ST_SKIP_BLANK; state != ST_QUIT; ) {
            switch (state) {
            case ST_SKIP_BLANK:
                if (readLine (stream, line)) {
                    if (!line.empty ()) {
                        subBuffer.clear ();
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
                    std::wistringstream lineStream (line);
                    lineStream >> index;

                    if (!lineStream.fail ()) {
                        state = ST_READ_TIMESTAMPS;
                    } else
                        state = ST_ERROR;
                }
                break;

            case ST_READ_TIMESTAMPS:
                if (readLine (stream, line))
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
                    std::wistringstream lineStream (line);

                    lineStream >>
                        hh1 >> sign1_1 >> mm1 >> sign1_2 >> ss1 >> sign1_3 >> mss1 >>
                        dash1 >> dash2 >> angle >>
                        hh2 >> sign2_1 >> mm2 >> sign2_2 >> ss2 >> sign2_3 >> mss2;

                    if (!lineStream.fail ()) {
                        int msecs1 = (hh1 * 3600000) + (mm1 * 60000) + (ss1 * 1000) + mss1;
                        int msecs2 = (hh2 * 3600000) + (mm2 * 60000) + (ss2 * 1000) + mss2;

                        subBuffer.timeBegin = msecs1;
                        subBuffer.timeEnd = msecs2;
                        subLineCount = 0;

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

                if ((readLine (stream, line)) && (!line.empty ())) {
                    ++subLineCount;
                    state = ST_PARSE_TEXT;
                } else {
                    if (subLineCount > 0) {
                        subtitles.push_back (subBuffer);
                        state = ST_SKIP_BLANK;
                    } else
                        state = ST_ERROR;
                }
                break;

            case ST_PARSE_TEXT:
                if (line.compare (L"\\n") != 0)
                    subBuffer.lines.push_back (line);
                else
                    subBuffer.lines.push_back (WString ());
                state = ST_READ_TEXT;
                break;

            case ST_ERROR:
                state = ST_QUIT;
                break;
            }

            if (stream.eof ())
                state = ST_QUIT;
        }

        return subtitles;
    }


    // (static)
    bool SrtParser::readLine (
        std::istream& stream,
        WString& line)
    {
        String buffer;

        std::getline (stream, buffer);

        line = StringHelper::toWString (buffer);

        return !stream.fail ();
    }


} // namespace bbi
