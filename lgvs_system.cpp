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


#include "lgvs_system.h"

#include <algorithm>

#include <windows.h>
#include <shlwapi.h>


namespace lgvs {


// (static)
std::wstring System::get_current_dir()
{
    const DWORD MAX_BUFFER_SIZE = MAX_PATH;
    WCHAR buffer[MAX_BUFFER_SIZE];
    DWORD api_result = 0;

    api_result = ::GetCurrentDirectoryW(MAX_BUFFER_SIZE, buffer);

    if (api_result > 0 && api_result < MAX_BUFFER_SIZE)
        return std::wstring(buffer, api_result);

    return std::wstring();
}

// (static)
std::wstring System::get_system_dir()
{
    const DWORD MAX_BUFFER_SIZE = MAX_PATH;
    WCHAR buffer[MAX_BUFFER_SIZE];
    DWORD api_result = 0;

    api_result = ::GetSystemDirectoryW(buffer, MAX_BUFFER_SIZE);

    if (api_result > 0 && api_result < MAX_BUFFER_SIZE)
        return std::wstring(buffer, api_result);

    return std::wstring();
}

// (static)
bool System::is_file_exists(const std::wstring& path)
{
    bool result = false;
    WIN32_FIND_DATAW wfd;
    HANDLE fh = INVALID_HANDLE_VALUE;

    fh = ::FindFirstFileW(path.c_str(), &wfd);

    if (fh != INVALID_HANDLE_VALUE) {
        result = true;
        ::FindClose(fh);
    }

    return result;
}

// (static)
std::wstring System::combine_paths(
    const std::wstring& path1,
    const std::wstring& path2)
{
    if (path1.empty())
        return path2;

    if (path2.empty())
        return path1;

    std::wstring result;
    result.reserve(path1.size() + path2.size() + 1);
    result = path1;

    if (!is_path_separator (result[result.size() - 1]))
        result += L'\\';

    result += path2;

    class SlashPredicate {
    public:
        static bool is_slash(wchar_t ch)
        {
            return ch == L'/';
        }
    }; // class SlashPredicate

    std::replace_if(result.begin(), result.end(),
        SlashPredicate::is_slash, L'\\');

    return result;
}

// (static)
std::wstring System::combine_paths(
    const std::wstring& path1,
    const std::wstring& path2,
    const std::wstring& path3)
{
    return combine_paths(combine_paths(path1, path2), path3);
}

// (static)
bool System::is_path_separator(wchar_t ch)
{
    return ch == L'\\' || ch == L'/';
}

// (static)
std::wstring System::extract_file_name(const std::wstring& path)
{
    std::wstring::size_type pos = path.find_last_of(L"\\/");

    if (pos != path.npos)
        return path.substr(pos + 1);

    return path;
}

// (static)
std::wstring System::change_file_extension(
    const std::wstring& path,
    const std::wstring& new_extension)
{
    if (new_extension.size() <= 1)
        return path;

    std::wstring::size_type pos = path.find_last_of(L'.');

    if (pos != path.npos) {
        std::wstring new_path(path);
        new_path.erase(pos);
        new_path += new_extension;
        return new_path;
    }

    return path;
}


} // namespace lgvs
