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



#include <map>
#include <sstream>
#include <string>


namespace lgvs {


class Config {
public:
    Config();
    Config(const Config& that);
    ~Config();
    Config& operator=(const Config& that);

    bool load_from_file(const std::wstring& file_name);

    const std::wstring& get_string(
        const std::wstring& key,
        const std::wstring& default_value = std::wstring());

    float get_float(
        const std::wstring& key,
        float default_value);

    float get_clamped_float(
        const std::wstring& key,
        float default_value,
        float min_value,
        float max_value);

    float get_percents(
        const std::wstring& key,
        float default_percents,
        float default_value,
        bool is_default_percents,
        bool& is_percents);

    float get_clamped_percents(
        const std::wstring& key,
        float default_percents,
        float min_percents_value,
        float max_percents_value,
        float default_value,
        float min_value,
        float max_value,
        bool is_default_percents,
        bool& is_percents);

    unsigned get_rgba_color(
        const std::wstring& key,
        unsigned default_value);

private:
    typedef std::map<std::wstring,std::wstring> ConfigMap;
    typedef ConfigMap::iterator ConfigMapIt;
    typedef ConfigMap::const_iterator ConfigMapCIt;

    ConfigMap config_map_;
    std::wistringstream sstream_;

    bool parse_line(
        const std::wstring& line,
        std::wstring& parameter,
        std::wstring& value);
}; // class Config


} // namespace lgvs
