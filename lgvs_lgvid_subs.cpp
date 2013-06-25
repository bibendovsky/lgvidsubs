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


#include "lgvs_lgvid_subs.h"

#include "lgvs_globals.h"
#include "lgvs_math.h"
#include "lgvs_srt_parser.h"
#include "lgvs_string_helper.h"
#include "lgvs_system.h"


namespace lgvs {


const std::wstring LgVidSubs::BLANK_WSTRING = L"";
const std::wstring LgVidSubs::DEFAULT_FONT_FAMILY = L"Arial";


LgVidSubs::LgVidSubs () :
    refresh_video(false),
    refresh_subtitle(false),
    host_(nullptr),
    config_(),
    font_filename_(),
    has_subtitles_(false)
{
}

LgVidSubs::~LgVidSubs()
{
    uninitialize();
}

void LgVidSubs::read_config_values()
{
    //
    auto font_family = config_.get_string(L"subs_font_family");
    if (font_family.empty())
        font_family = L"Arial";
    Globals::font_family = font_family;

    //
    Globals::font_size = config_.get_clamped_percents(
        L"subs_font_size",
        Globals::DEFAULT_FONT_SIZE_PCT,
        Globals::MIN_FONT_SIZE_PCT,
        Globals::MAX_FONT_SIZE_PCT,
        Globals::DEFAULT_FONT_SIZE_PIX,
        Globals::MIN_FONT_SIZE_PIX,
        Globals::MAX_FONT_SIZE_PIX,
        true,
        Globals::font_size_in_percents);

    //
    Globals::font_weight = config_.get_clamped_float(
        L"subs_font_weight",
        Globals::DEFAULT_FONT_WEIGHT,
        Globals::MIN_FONT_WEIGHT,
        Globals::MAX_FONT_WEIGHT);

    //
    Globals::font_color = config_.get_rgba_color(
        L"subs_font_color",
        Globals::DEFAULT_FONT_COLOR);

    //
    Globals::shadow_color = config_.get_rgba_color(
        L"subs_shadow_color",
        Globals::DEFAULT_SHADOW_COLOR);

    //
    Globals::shadow_offset_x = config_.get_clamped_percents(
        L"subs_shadow_offset_x",
        Globals::DEFAULT_SHADOW_OFFSET_PCT,
        Globals::MIN_SHADOW_OFFSET_PCT,
        Globals::MAX_SHADOW_OFFSET_PCT,
        Globals::DEFAULT_SHADOW_OFFSET_PIX,
        Globals::MIN_SHADOW_OFFSET_PIX,
        Globals::MAX_SHADOW_OFFSET_PIX,
        true,
        Globals::shadow_offset_x_in_percents);

    //
    Globals::shadow_offset_y = config_.get_clamped_percents(
        L"subs_shadow_offset_y",
        Globals::DEFAULT_SHADOW_OFFSET_PCT,
        Globals::MIN_SHADOW_OFFSET_PCT,
        Globals::MAX_SHADOW_OFFSET_PCT,
        Globals::DEFAULT_SHADOW_OFFSET_PIX,
        Globals::MIN_SHADOW_OFFSET_PIX,
        Globals::MAX_SHADOW_OFFSET_PIX,
        true,
        Globals::shadow_offset_y_in_percents);

    //
    Globals::space_after = config_.get_clamped_percents(
        L"subs_space_after",
        Globals::DEFAULT_SPACE_AFTER_PCT,
        Globals::MIN_SPACE_AFTER_PCT,
        Globals::MAX_SPACE_AFTER_PCT,
        Globals::DEFAULT_SPACE_AFTER_PIX,
        Globals::MIN_SPACE_AFTER_PIX,
        Globals::MAX_SPACE_AFTER_PIX,
        true,
        Globals::space_after_in_percents);
}

WStringList LgVidSubs::read_movie_paths()
{
    WStringList result;
    auto cur_dir = System::get_current_dir();

    if (cur_dir.empty())
        return result;

    auto config_path = System::combine_paths(
        cur_dir, L"install.cfg");

    std::ifstream stream(config_path.c_str());

    if (!stream.is_open())
        return result;

    std::string line;
    auto movie_path_index = std::string::npos;

    while (std::getline(stream, line)) {
        auto comment_index = line.find_first_of(';');

        if (comment_index != line.npos)
            line.erase(comment_index);

        movie_path_index = line.find("movie_path");

        if (movie_path_index != line.npos)
            break;
    }

    if (movie_path_index == line.npos)
        return result;

    auto length = line.size();
    auto path_index = movie_path_index + 10;

    for ( ; path_index < length; ++path_index) {
        if (line[path_index] != ' ' && line[path_index] != '\t')
            break;
    }

    auto index = path_index;

    while (index < length) {
        auto plus_index = line.find_first_of('+', index);

        if (plus_index == line.npos)
            plus_index = length;

        auto path_length = plus_index - index;

        if (path_length == 0)
            break;

        result.push_back(StringHelper::to_wstring(
            line.substr(index, path_length)));

        index += path_length + 1;
    }

    return result;
}

bool LgVidSubs::check_subtitle(double pts)
{
    // Predicate to search subtitle within a specified period (ms).
    class SubtitlePred {
    public:
        SubtitlePred(int time) :
            time_(time)
        {
        }

        bool operator()(const Subtitle& subtitle)
        {
            return
                time_ >= subtitle.time_begin &&
                time_ <= subtitle.time_end;
        }

    private:
        int time_;
    }; // class SubtitlePred


    int time = static_cast<int>(pts * 1000.0);

    auto it = std::find_if(
        Globals::subs.begin(),
        Globals::subs.end(),
        SubtitlePred(time));

    if (it != Globals::subs.end()) {
        Globals::sub_index = it - Globals::subs.begin();
        return true;
    }

    Globals::sub_index = -1;
    return false;
}

void LgVidSubs::initialize(ILGVideoDecoderHost* host, const char* file_path)
{
    uninitialize();

    if (host == nullptr)
        return;

    host_ = host;

    print_log("Subtitles Init...");

    if (file_path == nullptr) {
        host_ = nullptr;
        print_log("\tNull filename.");
        return;
    }

    auto subs_file_path = StringHelper::to_wstring(file_path);
    auto subs_file_name = System::extract_file_name(subs_file_path);
    auto subs_new_file_name = System::change_file_extension(
        subs_file_name, L".srt");

    std::ifstream srt_stream;
    auto cur_dir = System::get_current_dir();
    auto movie_paths = read_movie_paths();

    for (std::wstring::size_type i = 0; i < movie_paths.size(); ++i) {
        auto subs_tmp_path = System::combine_paths(
            cur_dir, movie_paths[i], subs_new_file_name);

        srt_stream.close();
        srt_stream.clear();
        srt_stream.open(subs_tmp_path.c_str());

        if (srt_stream.is_open())
            break;
    }

    if (srt_stream.is_open())
        Globals::subs = SrtParser::parse(srt_stream);
    else
        Globals::subs.clear();

    auto config_file_name = System::combine_paths(
        System::get_current_dir(), L"cam_ext.cfg");

    config_.load_from_file(config_file_name);

    if (!Globals::subs.empty()) {
        host_ = host;

        font_filename_ = config_.get_string(L"subs_font_filename");

        if (!font_filename_.empty()) {
            if (::AddFontResourceW(font_filename_.c_str()) == 0) {
                auto msg = L"\tFailed to register a font: " +
                    font_filename_ + L".";
                print_log(msg);
                font_filename_.clear();
            }
        }

        if (!D3d9Renderer::initialize()) {
            Globals::subs.clear();
            print_log(L'\t' + D3d9Renderer::get_error_string());
        }

        read_config_values();

        Globals::sub_index = -1;
        Globals::enable_subs = true;
    }

    if (!Globals::subs.empty())
        has_subtitles_ = true;
    else
        uninitialize();
}

void LgVidSubs::uninitialize()
{
    D3d9Renderer::uninitialize();

    refresh_video = false;

    host_ = nullptr;

    if (!font_filename_.empty()) {
        ::RemoveFontResourceW(font_filename_.c_str());
        font_filename_.clear();
    }

    has_subtitles_ = false;
}

bool LgVidSubs::has_subtitles() const
{
    return has_subtitles_;
}

void LgVidSubs::print_log(const char* text)
{
    if (host_ == nullptr)
        return;

    if (text == nullptr)
        text = "";

    host_->LogPrint(text);
}

void LgVidSubs::print_log(const std::string& text)
{
    if (host_ == nullptr)
        return;

    host_->LogPrint(text.c_str());
}

void LgVidSubs::print_log(const std::wstring& text)
{
    print_log(StringHelper::to_string(text));
}


} // namespace lgvs
