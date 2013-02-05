#include "bbi_subtitle.h"


namespace bbi {


    Subtitle::Subtitle () :
        time_begin (0),
        time_end (0),
        lines ()
    {
    }

    Subtitle::Subtitle (const Subtitle& that) :
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
            time_begin = that.time_begin;
            time_end = that.time_end;
            lines = that.lines;
        }

        return *this;
    }

    void Subtitle::clear ()
    {
        time_begin = 0;
        time_end = 0;
        lines.clear ();
    }


} // namespace bbi
