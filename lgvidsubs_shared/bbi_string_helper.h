#ifndef BBI_STRING_HELPER_H
#define BBI_STRING_HELPER_H


#include "bbi_types.h"


namespace bbi {


class StringHelper {
public:
    static WString to_wstring (const String& string);

    static void copy_w_to_c (const WString& w_string, wchar_t* c_string,
        size_t max_length);


private:
    StringHelper ();

    StringHelper (const StringHelper& that);

    ~StringHelper ();

    StringHelper& operator = (const StringHelper& that);
}; // class StringHelper


} // namespace bbi


#endif // BBI_STRING_HELPER_H
