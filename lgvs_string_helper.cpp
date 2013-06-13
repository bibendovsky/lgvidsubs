/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 */


#include "lgvs_string_helper.h"

#include <locale>
#include <memory>


namespace {


typedef std::wstring::size_type size_type;
typedef std::codecvt<wchar_t,char,mbstate_t> CodeCvt;
typedef std::ctype<wchar_t> WCType;


const CodeCvt& local8_facet()
{
    static std::locale local8_locale("");

    static const CodeCvt& result =
        std::use_facet<CodeCvt>(local8_locale);

    return result;
};

const WCType& wctype_facet()
{
    static std::locale locale("");

    static const WCType& result =
        std::use_facet<WCType>(locale);

    return result;
}

template<class Facet>
std::wstring& convert_from(std::wstring& string, const char* src_chars,
    size_type src_length, const Facet& facet)
{
    const int BUFFER_SIZE = 16;

    string.clear();
    mbstate_t state = { 0, };
    wchar_t buffer[BUFFER_SIZE];
    const char* next1;
    wchar_t* next2;

    const char* i;
    const char* n;
    
    for (i = src_chars, n = i + src_length; i < n; ) {
        std::codecvt_base::result out_result = facet.in(
            state, i, n, next1, buffer, buffer + BUFFER_SIZE, next2);

        if (out_result != std::codecvt_base::error) {
            i = next1;
            string.append(buffer, next2);
        } else {
            ++i;
            string.append(1, L'?');
        }
    }

    return string;
}

template<class Facet>
std::string& convert_to(std::string& string, const wchar_t* src_chars,
    size_type src_length, const Facet& facet)
{
    const int BUFFER_SIZE = 16;

    string.clear();
    mbstate_t state = { 0, };
    char buffer[BUFFER_SIZE];
    const wchar_t* next1;
    char* next2;

    const wchar_t* i;
    const wchar_t* n;
    
    for (i = src_chars, n = i + src_length; i < n; ) {
        std::codecvt_base::result out_result = facet.out(
            state, i, n, next1, buffer, buffer + BUFFER_SIZE, next2);

        if (out_result != std::codecvt_base::error) {
            i = next1;
            string.append(buffer, next2);
        } else {
            ++i;
            string.append(1, '?');
        }
    }

    return string;
}



} // namespace


namespace lgvs {


// (static)
std::string StringHelper::to_string(const std::wstring& string)
{
    if (string.empty())
        return std::string ();

    std::string result;

    return convert_to(result, string.c_str(), string.size(),
        local8_facet());
}

// (static)
std::wstring StringHelper::to_wstring(const std::string& string)
{
    if (string.empty())
        return std::wstring();

    std::wstring result;

    return convert_from(result, string.c_str(), string.size(),
        local8_facet());
}

// (static)
void StringHelper::copy_w_to_c(const std::wstring& w_string, wchar_t* c_string,
    size_t max_length)
{
    if (c_string == NULL)
        return;


    size_t length = std::min(w_string.size(), max_length);

    std::wstring::traits_type::copy(c_string, w_string.c_str(), length);

    c_string[length] = L'\0';
}

// (static)
bool StringHelper::is_empty_or_white_space(const std::wstring& string)
{
    return string.find_first_not_of(L" \f\n\r\t\v") == string.npos;
}

// (static)
std::wstring StringHelper::to_lower(const std::wstring& string)
{
    std::wstring result(string);
    wctype_facet().tolower(&result[0], &result[result.size()]);
    return result;
}

// (static)
const std::wstring& StringHelper::get_empty()
{
    static std::wstring result;
    return result;
}


} // namespace lgvs
