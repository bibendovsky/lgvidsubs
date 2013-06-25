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


#ifndef LGVS_DIRECTORY_H
#define LGVS_DIRECTORY_H


#include <string>


namespace lgvs {


class System {
public:
    static std::wstring get_current_dir();

    static std::wstring get_system_dir();

    static bool is_file_exists(const std::wstring& path);

    static std::wstring combine_paths(
        const std::wstring& path1,
        const std::wstring& path2);

    static std::wstring combine_paths(
        const std::wstring& path1,
        const std::wstring& path2,
        const std::wstring& path3);

    static bool is_path_separator(wchar_t ch);

    static std::wstring extract_file_name(const std::wstring& path);

    // Note: Prefix a new extension with a dot.
    static std::wstring change_file_extension(
        const std::wstring& path,
        const std::wstring& new_extension);

    static size_t get_page_size();


private:
    System();
    System(const System& that);
    ~System();
    System& operator=(const System& that);
}; // class System


} // namespace lgvs


#endif // LGVS_DIRECTORY_H
