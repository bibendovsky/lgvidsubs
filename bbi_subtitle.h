#ifndef BBI_SUBTITLE_H
#define BBI_SUBTITLE_H


#include "bbi_types.h"


namespace bbi {


// Container for a one subtitle item.
class Subtitle {
public:
    int time_begin; // time (ms) when to show the subtitle
    int time_end; // time (ms) when to hide the subtitle
    WStringList lines; // text of the subtitle

    Subtitle ();

    Subtitle (const Subtitle& that);

    ~Subtitle ();

    Subtitle& operator = (const Subtitle& that);


    // Clears all data.
    void clear ();
}; // class Subtitle


typedef std::vector<Subtitle> SubtitleList;


} // namespace bbi


#endif // BBI_SUBTITLE_H
