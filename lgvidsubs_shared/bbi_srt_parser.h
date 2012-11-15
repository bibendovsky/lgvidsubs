#ifndef BBI_SRT_PARSER_H
#define BBI_SRT_PARSER_H


//
// Parser for .srt subtitles.
//


#include <iostream>

#include "bbi_subtitle.h"


namespace bbi {


    class SrtParser {
    public:
        static SubtitleList parse (
            std::istream& stream);

    private:
        SrtParser ();

        SrtParser (
            const SrtParser& that);

        ~SrtParser ();

        SrtParser& operator = (
            const SrtParser& that);

        // Reads a one line from a stream.
        static bool readLine (
            std::istream& stream,
            WString& line);
    }; // class SrtParser


} // namespace bbi


#endif // BBI_SRT_PARSER_H
