#include "bbi_subtitle.h"


namespace bbi {


    Subtitle::Subtitle () :
        timeBegin (0),
        timeEnd (0),
        lines ()
    {
    }

    Subtitle::Subtitle (
        const Subtitle& that) :
        timeBegin (that.timeBegin),
        timeEnd (that.timeEnd),
        lines (that.lines)
    {
    }

    Subtitle::~Subtitle ()
    {
    }

    Subtitle& Subtitle::operator = (
        const Subtitle& that)
    {
        if (&that != this) {
            timeBegin = that.timeBegin;
            timeEnd = that.timeEnd;
            lines = that.lines;
        }

        return *this;
    }

    void Subtitle::clear ()
    {
        timeBegin = 0;
        timeEnd = 0;
        lines.clear ();
    }


} // namespace bbi
