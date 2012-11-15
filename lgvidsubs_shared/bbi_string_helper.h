#ifndef BBI_STRING_HELPER_H
#define BBI_STRING_HELPER_H


#include "bbi_types.h"


namespace bbi {


    class StringHelper {
    public:
        static WString toWString (
            const String& string);

        static void copyWToC (
            const WString& wString,
            wchar_t* cString,
            size_t maxLength);


    private:
        StringHelper ();

        StringHelper (
            const StringHelper& that);

        ~StringHelper ();

        StringHelper& operator = (
            const StringHelper& that);
    }; // class StringHelper


} // namespace bbi


#endif // BBI_STRING_HELPER_H
