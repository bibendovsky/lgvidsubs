#include <locale>
#include <memory>

#include "bbi_string_helper.h"


namespace {


    typedef std::codecvt<wchar_t,char,mbstate_t> WCCodeCvt;


    const std::locale& oemLocale ()
    {
        static std::locale result (".ACP");
        return result;
    }

    const WCCodeCvt& wcCodeCvt ()
    {
        const WCCodeCvt& result = std::use_facet<WCCodeCvt> (oemLocale ());
        return result;
    }


} // namespace


namespace bbi {


    // (static)
    WString StringHelper::toWString (
        const String& string)
    {
        if (string.empty ())
            return WString ();

        WCCodeCvt::state_type state (0);

        int length = wcCodeCvt ().length (
            state, &string[0], &string[0] + string.length (), string.length ());

        std::auto_ptr<wchar_t> buffer (new wchar_t[length + 1]);

        if (buffer.get () == 0)
            return WString ();

        const char* beyondC = 0;
        wchar_t* beyondW = 0;

        WCCodeCvt::result cvtResult = wcCodeCvt ().in (
            state,
            string.c_str (),
            string.c_str () + string.length (),
            beyondC,
            buffer.get (),
            buffer.get () + length,
            beyondW);

        if (cvtResult != WCCodeCvt::ok)
            return WString ();

        return WString (buffer.get (), length);
    }

    // (static)
    void StringHelper::copyWToC (
        const WString& wString,
        wchar_t* cString,
        size_t maxLength)
    {
        if (cString == 0)
            return;


        size_t length = std::min (wString.size (), maxLength);

        WString::traits_type::copy (cString, wString.c_str (), length);

        cString[length] = '\0';
    }


} // namespace bbi
