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


#ifndef LGVS_STRING_HELPER_H
#define LGVS_STRING_HELPER_H


#include <string>


namespace lgvs {


class StringHelper {
public:
    static std::string to_string(const std::wstring& string);

    static std::wstring to_wstring(const std::string& string);

    static void copy_w_to_c(const std::wstring& w_string,
        wchar_t* c_string, size_t max_length);

    static bool is_empty_or_white_space(const std::wstring& string);

    static std::wstring to_lower(const std::wstring& string);

    static const std::wstring& get_empty();


private:
    StringHelper();
    StringHelper(const StringHelper& that);
    ~StringHelper();
    StringHelper& operator=(const StringHelper& that);
}; // class StringHelper


} // namespace lgvs


#endif // LGVS_STRING_HELPER_H
