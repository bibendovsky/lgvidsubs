#include <locale>
#include <memory>

#include "bbi_string_helper.h"


namespace {


typedef std::wstring::size_type size_type;
typedef std::codecvt<wchar_t,char,mbstate_t> CodeCvt;


const CodeCvt& local8_facet ()
{
    static std::locale local8_locale ("");

    static const CodeCvt& result =
        std::use_facet<CodeCvt> (local8_locale);

    return result;
};

template<class Facet>
bbi::WString& convert_from (bbi::WString& string, const char* src_chars,
    size_type src_length, const Facet& facet)
{
    const int BUFFER_SIZE = 16;

    string.clear ();
    mbstate_t state = { 0, };
    wchar_t buffer[BUFFER_SIZE];
    const char* next1;
    wchar_t* next2;

    const char* i;
    const char* n;
    
    for (i = src_chars, n = i + src_length; i < n; ) {
        std::codecvt_base::result out_result = facet.in (
            state, i, n, next1, buffer, buffer + BUFFER_SIZE, next2);

        if (out_result != std::codecvt_base::error) {
            i = next1;
            string.append (buffer, next2);
        } else {
            ++i;
            string.append (1, L'?');
        }
    }

    return string;
}


} // namespace


namespace bbi {


// (static)
WString StringHelper::to_wstring (const String& string)
{
    if (string.empty ())
        return WString ();

    WString result;

    return convert_from (result, string.c_str (), string.size (),
        local8_facet ());
}

// (static)
void StringHelper::copy_w_to_c (const WString& w_string, wchar_t* c_string,
    size_t max_length)
{
    if (c_string == NULL)
        return;


    size_t length = std::min (w_string.size (), max_length);

    WString::traits_type::copy (c_string, w_string.c_str (), length);

    c_string[length] = L'\0';
}


} // namespace bbi
