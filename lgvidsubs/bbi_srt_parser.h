#ifndef BBI_SRT_PARSER_H
#define BBI_SRT_PARSER_H


//
// Parser for .srt subtitles.
//


#include <iostream>
#include <string>
#include <vector>


typedef std::vector<std::string> StringList;


// Container for a one subtitle item.
class Subtitle {
public:
    int timeBegin; // time (ms) when to show the subtitle
    int timeEnd; // time (ms) when to hide the subtitle
    StringList lines; // text of the subtitle

    Subtitle () :
        timeBegin (0),
        timeEnd (0),
        lines ()
    {
    }

    Subtitle (
        const Subtitle& that) :
        timeBegin (that.timeBegin),
        timeEnd (that.timeEnd),
        lines (that.lines)
    {
    }

    ~Subtitle ()
    {
    }

    Subtitle& operator = (
        const Subtitle& that)
    {
        if (&that != this) {
            timeBegin = that.timeBegin;
            timeEnd = that.timeEnd;
            lines = that.lines;
        }

        return *this;
    }

    // Clears all data.
    void clear ()
    {
        timeBegin = 0;
        timeEnd = 0;
        lines.clear ();
    }
}; // class Subtitle

typedef std::vector<Subtitle> SubtitleList;


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
        std::string& line);
}; // class SrtParser


#endif // BBI_SRT_PARSER_H
