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


#include "lgvs_config.h"

#include <fstream>
#include <sstream>

#include "lgvs_math.h"
#include "lgvs_string_helper.h"


namespace lgvs {


Config::Config() :
    config_map_(),
    sstream_()
{
}

Config::Config(const Config& that) :
    config_map_(that.config_map_),
    sstream_()
{
}

Config::~Config()
{
}

Config& Config::operator=(const Config& that)
{
    if (&that != this)
        config_map_ = that.config_map_;

    return *this;
}

bool Config::load_from_file(const std::wstring& file_name)
{
    config_map_.clear();

    if (StringHelper::is_empty_or_white_space(file_name))
        return false;

    std::ifstream stream(file_name.c_str());

    if (!stream.is_open())
        return false;

    for (std::string line; std::getline(stream, line); ) {
        auto wline = StringHelper::to_wstring(line);

        std::wstring parameter;
        std::wstring value;

        if (parse_line(wline, parameter, value))
            config_map_[parameter] = value;
    }

    return true;
}

const std::wstring& Config::get_string(
    const std::wstring& key,
    const std::wstring& default_value)
{
    auto lkey = StringHelper::to_lower(key);
    auto it = config_map_.find(lkey);

    if (it != config_map_.end())
        return it->second;

    return default_value;
}

float Config::get_float(
    const std::wstring& key,
    float default_value)
{
    sstream_.clear();
    sstream_.str(get_string(key));

    float value;
    sstream_ >> value;

    if (!sstream_)
        return default_value;

    auto any_non_space = L'\0';
    sstream_ >> any_non_space;

    if (!sstream_)
        return default_value;

    return value;
}

float Config::get_clamped_float(
    const std::wstring& key,
    float default_value,
    float min_value,
    float max_value)
{
    auto value = get_float(key, default_value);
    return Math::clamp(value, min_value, max_value);
}

float Config::get_percents(
    const std::wstring& key,
    float default_percents,
    float default_value,
    bool is_default_percents,
    bool& is_percents)
{
    is_percents = is_default_percents;
    auto default_result = is_percents ? default_percents : default_value;

    sstream_.clear();
    sstream_.str(get_string(key));

    // Get float value.
    float value;
    sstream_ >> value;

    // On fail return default value.
    if (!sstream_)
        return default_result;

    // Is there a percent sign?
    auto percents = L'\0';
    sstream_ >> percents;

    if (!sstream_) {
        // Not percents, return a parsed value.
        is_percents = false;
        return value;
    }

    if (percents != L'%') {
        // Found symbol but not percents, return a default value.
        return default_result;
    }

    // Is there any non whites pace symbol behind a percents sign?
    auto any_non_space = L'\0';
    sstream_ >> any_non_space;

    if (!sstream_) {
        // Percents sign was a last one, return a parsed value.
        is_percents = true;
        return value;
    }

    // Found a non white space symbol, return a default value.
    return default_result;
}

float Config::get_clamped_percents(
    const std::wstring& key,
    float default_percents,
    float min_percents_value,
    float max_percents_value,
    float default_value,
    float min_value,
    float max_value,
    bool is_default_percents,
    bool& is_percents)
{
    auto value = get_percents(
        key,
        default_percents,
        default_value,
        is_default_percents,
        is_percents);

    if (is_percents)
        return Math::clamp(value, min_percents_value, max_percents_value);
    else
        return Math::clamp(value, min_value, max_value);
}

unsigned Config::get_rgba_color(
    const std::wstring& key,
    unsigned default_value)
{
    sstream_.clear();
    sstream_.str(get_string(key));

    float color_buffer[4];

    sstream_ >>
        color_buffer[0] >> color_buffer[1] >>
        color_buffer[2] >> color_buffer[3];

    if (!sstream_)
        return default_value;

    auto r = static_cast<unsigned>(
        255.0F * Math::clamp(color_buffer[0], 0.0F, 1.0F));
    auto g = static_cast<unsigned>(
        255.0F * Math::clamp(color_buffer[1], 0.0F, 1.0F));
    auto b = static_cast<unsigned>(
        255.0F * Math::clamp(color_buffer[2], 0.0F, 1.0F));
    auto a = static_cast<unsigned>(
        255.0F * Math::clamp(color_buffer[3], 0.0F, 1.0F));

    return
        ((a & 0xFF) << 24) |
        ((r & 0xFF) << 16) |
        ((g & 0xFF) <<  8) |
        ((b & 0xFF) <<  0);
}

bool Config::parse_line(
    const std::wstring& line,
    std::wstring& parameter,
    std::wstring& value)
{
    if (StringHelper::is_empty_or_white_space(line))
        return false;

    size_t begin_pos;
    size_t end_pos;
    size_t comment_pos;

    static const auto white_spaces = L" \t\v\f\r\n";

    parameter.clear();
    value.clear();

    // Search for beginning of a parameter.
    begin_pos = line.find_first_not_of(white_spaces);

    if (begin_pos == line.npos)
        return false;

    // Search for ending of the parameter.
    end_pos = line.find_first_of(white_spaces, begin_pos);

    // Search for a possible comment delimiter inside the parameter.
    comment_pos = line.find_first_of(L';', begin_pos);

    if (comment_pos >= begin_pos && comment_pos <= end_pos)
        end_pos = comment_pos;

    // Extract the parameter.
    parameter = line.substr(begin_pos, end_pos - begin_pos);
    parameter = StringHelper::to_lower(parameter);

    if (parameter.empty())
        return false;

    // Search for beginning of a value.
    begin_pos = line.find_first_not_of(white_spaces, end_pos);

    if (begin_pos == line.npos)
        return true;

    // Search for ending of the value.
    end_pos = line.find_last_not_of(white_spaces);

    if (end_pos != line.npos)
        ++end_pos;

    // Search for a possible comment delimiter inside parameter.
    comment_pos = line.find_first_of(L';', begin_pos);

    if (comment_pos >= begin_pos && comment_pos <= end_pos)
        end_pos = comment_pos;

    // Extract the value.
    value = line.substr(begin_pos, end_pos - begin_pos);

    return true;
}


} // namespace lgvs
