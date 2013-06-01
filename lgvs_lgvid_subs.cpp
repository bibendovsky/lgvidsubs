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

#include "lgvs_math.h"
#include "lgvs_srt_parser.h"
#include "lgvs_string_helper.h"


namespace lgvs {


const std::wstring LgVidSubs::BLANK_WSTRING = L"";
const std::wstring LgVidSubs::DEFAULT_FONT_FAMILY = L"Arial";


LgVidSubs::LgVidSubs () :
    refresh_video (false),
    host_ (NULL),
    f_set_info_ (NULL),
    f_set_font_info_ (NULL),
    f_set_subs_ (NULL),
    f_show_sub_ (NULL),
    f_enable_subs_ (NULL),
    ss_buffer_ (),
    g_info_ (),
    g_font_info_ (),
    g_subs_ (),
    g_sub_index_ (-1),
    g_enable_subs_ (false),
    subtitles_ (),
    d3d9_library_ (NULL),
    font_filename_ (),
    has_subtitles_ (false),
    last_sub_index_ (-1)
{
}

LgVidSubs::~LgVidSubs ()
{
    uninitialize ();
}

std::wstring LgVidSubs::read_config_string (const char* key_name,
    ILGVideoDecoderHost* host, const std::wstring& default_value)
{
    const int VALUE_BUFFER_SIZE = 128;
    const int VALUE_BUFFER_LENGTH = VALUE_BUFFER_SIZE - 1;
    char value_buffer[VALUE_BUFFER_SIZE];

    if (!host->GetConfigValue (key_name, value_buffer, VALUE_BUFFER_LENGTH))
        return default_value;

    value_buffer[VALUE_BUFFER_LENGTH] = '\0';

    return lgvs::StringHelper::to_wstring (value_buffer);
}

float LgVidSubs::read_config_float_value (const char* key_name,
    ILGVideoDecoderHost* host, float default_value)
{
    ss_buffer_.clear ();
    ss_buffer_.str (read_config_string (key_name, host));

    float value;
    ss_buffer_ >> value;

    if (ss_buffer_.fail ())
        return default_value;

    wchar_t any_non_space = L'\0';
    ss_buffer_ >> any_non_space;

    if (!ss_buffer_.fail ())
        return default_value;

    return value;
}

float LgVidSubs::read_config_percents_value (const char* key_name,
    ILGVideoDecoderHost* host, float default_value, bool default_is_percents,
    bool& is_percents)
{
    is_percents = default_is_percents;

    ss_buffer_.clear ();
    ss_buffer_.str (read_config_string (key_name, host));

    float value;
    ss_buffer_ >> value;

    if (ss_buffer_.fail ())
        return default_value;

    wchar_t percents = L'\0';
    ss_buffer_ >> percents;

    if (ss_buffer_.fail ()) {
        default_is_percents = false;
        return value;
    }

    wchar_t any_non_space = L'\0';
    ss_buffer_ >> any_non_space;

    if (!ss_buffer_.fail ())
        return default_value;

    return value;
}

unsigned LgVidSubs::read_color_value (const char* key_name,
    ILGVideoDecoderHost* host, unsigned default_value)
{
    ss_buffer_.clear ();
    ss_buffer_.str (read_config_string (key_name, host));

    float color_buffer[4];

    ss_buffer_ >> color_buffer[0] >> color_buffer[1] >>
        color_buffer[2] >> color_buffer[3];

    if (ss_buffer_.fail ())
        return default_value;

    unsigned r = static_cast<unsigned> (
        255.0F * lgvs::Math::clamp (color_buffer[0], 0.0F, 1.0F));
    unsigned g = static_cast<unsigned> (
        255.0F * lgvs::Math::clamp (color_buffer[1], 0.0F, 1.0F));
    unsigned b = static_cast<unsigned> (
        255.0F * lgvs::Math::clamp (color_buffer[2], 0.0F, 1.0F));
    unsigned a = static_cast<unsigned> (
        255.0F * lgvs::Math::clamp (color_buffer[3], 0.0F, 1.0F));

    return
        ((a & 0xFF) << 24) |
        ((r & 0xFF) << 16) |
        ((g & 0xFF) <<  8) |
        ((b & 0xFF) <<  0);
}

void LgVidSubs::read_config_values (ILGVideoDecoderHost* host)
{
    //
    std::wstring font_family = read_config_string ("subs_font_family", host);
    if (font_family.empty ())
        font_family = L"Arial";
    g_font_info_.font_family = font_family;


    //
    g_font_info_.font_size = read_config_percents_value (
        "subs_font_size", host, GlobalFontInfo::DEFAULT_FONT_SIZE_PCT,
        true, g_font_info_.font_size_in_percents);

    if (g_font_info_.font_size_in_percents) {
        g_font_info_.font_size = lgvs::Math::clamp (
            g_font_info_.font_size,
            GlobalFontInfo::MIN_FONT_SIZE_PCT,
            GlobalFontInfo::MAX_FONT_SIZE_PCT);
    } else {
        g_font_info_.font_size = lgvs::Math::clamp (
            g_font_info_.font_size,
            GlobalFontInfo::MIN_FONT_SIZE_PIX,
            GlobalFontInfo::MAX_FONT_SIZE_PIX);
    }

    //
    g_font_info_.font_weight = read_config_float_value (
        "subs_font_weight", host, GlobalFontInfo::DEFAULT_FONT_WEIGHT);

    g_font_info_.font_weight = lgvs::Math::clamp (
        g_font_info_.font_weight,
        GlobalFontInfo::MIN_FONT_WEIGHT,
        GlobalFontInfo::MAX_FONT_WEIGHT);

    //
    g_font_info_.font_color = read_color_value (
        "subs_font_color", host, GlobalFontInfo::DEFAULT_FONT_COLOR);

    //
    g_font_info_.shadow_color = read_color_value (
        "subs_shadow_color", host, GlobalFontInfo::DEFAULT_SHADOW_COLOR);

    //
    g_font_info_.shadow_offset_x = read_config_percents_value (
        "subs_shadow_offset_x", host, GlobalFontInfo::DEFAULT_SHADOW_OFFSET_PCT,
        true, g_font_info_.shadow_offset_x_in_percents);

    if (g_font_info_.shadow_offset_x_in_percents) {
        g_font_info_.shadow_offset_x = lgvs::Math::clamp (
            g_font_info_.shadow_offset_x,
            GlobalFontInfo::MIN_SHADOW_OFFSET_PCT,
            GlobalFontInfo::MAX_SHADOW_OFFSET_PCT);
    } else {
        g_font_info_.shadow_offset_x = lgvs::Math::clamp (
            g_font_info_.shadow_offset_x,
            GlobalFontInfo::MIN_SHADOW_OFFSET_PIX,
            GlobalFontInfo::MAX_SHADOW_OFFSET_PIX);
    }

    //
    g_font_info_.shadow_offset_y = read_config_percents_value (
        "subs_shadow_offset_y", host, GlobalFontInfo::DEFAULT_SHADOW_OFFSET_PCT,
        true, g_font_info_.shadow_offset_y_in_percents);

    if (g_font_info_.shadow_offset_y_in_percents) {
        g_font_info_.shadow_offset_y = lgvs::Math::clamp (
            g_font_info_.shadow_offset_y,
            GlobalFontInfo::MIN_SHADOW_OFFSET_PCT,
            GlobalFontInfo::MAX_SHADOW_OFFSET_PCT);
    } else {
        g_font_info_.shadow_offset_y = lgvs::Math::clamp (
            g_font_info_.shadow_offset_y,
            GlobalFontInfo::MIN_SHADOW_OFFSET_PIX,
            GlobalFontInfo::MAX_SHADOW_OFFSET_PIX);
    }

    //
    g_info_.space_after = read_config_percents_value (
        "subs_space_after", host, GlobalInfo::DEFAULT_SPACE_AFTER_PCT,
        true, g_info_.space_after_in_percents);

    if (g_info_.space_after_in_percents) {
        g_info_.space_after = lgvs::Math::clamp (
            g_info_.space_after,
            GlobalInfo::MIN_SPACE_AFTER_PCT,
            GlobalInfo::MAX_SPACE_AFTER_PCT);
    } else {
        g_info_.space_after = lgvs::Math::clamp (
            g_info_.space_after,
            GlobalInfo::MIN_SPACE_AFTER_PIX,
            GlobalInfo::MAX_SPACE_AFTER_PIX);
    }
}

std::wstring LgVidSubs::sys_get_current_dir ()
{
    const DWORD MAX_DIR_SIZE = MAX_PATH;
    wchar_t dir_buffer[MAX_DIR_SIZE];

    DWORD api_result = 0;

    api_result = ::GetCurrentDirectoryW (MAX_DIR_SIZE, dir_buffer);

    if (api_result == 0 || api_result >= MAX_DIR_SIZE)
        return std::wstring ();

    return dir_buffer;
}

lgvs::WStringList LgVidSubs::read_movie_paths ()
{
    lgvs::WStringList result;
    std::wstring cur_dir = sys_get_current_dir ();

    if (cur_dir.empty ())
        return result;

    std::wstring config_path = cur_dir + L"\\install.cfg";

    std::ifstream stream (config_path.c_str ());

    if (!stream.is_open ())
        return result;

    std::string line;
    std::string::size_type movie_path_index = std::string::npos;

    while (!std::getline (stream, line).fail ()) {
        std::string::size_type comment_index = line.find_first_of (';');

        if (comment_index != std::string::npos)
            line.erase (comment_index);

        movie_path_index = line.find ("movie_path");

        if (movie_path_index != std::string::npos)
            break;
    }

    if (movie_path_index == std::string::npos)
        return result;

    std::string::size_type length = line.size ();
    std::string::size_type path_index = movie_path_index + 10;

    for ( ; path_index < length; ++path_index) {
        if (line[path_index] != ' ' && line[path_index] != '\t')
            break;
    }

    std::string::size_type index = path_index;

    while (index < length) {
        std::string::size_type plus_index = line.find_first_of ('+', index);

        if (plus_index == std::string::npos)
            plus_index = length;

        std::string::size_type path_length = plus_index - index;

        if (path_length == 0)
            break;

        result.push_back (lgvs::StringHelper::to_wstring (
            line.substr (index, path_length)));

        index += path_length + 1;
    }

    return result;
}

bool LgVidSubs::check_subtitle (double pts)
{
    // Predicate to search subtitle within a specified period (ms).
    class SubtitlePred {
    public:
        SubtitlePred (int time) :
            time_ (time)
        {
        }

        bool operator () (const lgvs::Subtitle& subtitle)
        {
            return (time_ >= subtitle.time_begin) &&
                (time_ <= subtitle.time_end);
        }

    private:
        int time_;
    }; // class SubtitlePred


    int time = static_cast<int> (pts * 1000.0);

    lgvs::SubtitleListCIt it = std::find_if (
        subtitles_.begin (), subtitles_.end (), SubtitlePred (time));

    if (it != subtitles_.end ()) {
        if (last_sub_index_ != it->index) {
            last_sub_index_ = it->index;
            f_show_sub_ (it->index);
            return true;
        }
    } else {
        if (last_sub_index_ >= 0) {
            last_sub_index_ = -1;
            f_show_sub_ (-1);
            return true;
        }
    }

    return false;
}

void LgVidSubs::initialize (ILGVideoDecoderHost* host, const char* filename)
{
    uninitialize ();

    if (host == NULL)
        return;

    host_ = host;

    print_log ("Subtitles Init...");

    if (filename == NULL) {
        host_ = NULL;
        print_log ("\tNull filename.");
        return;
    }

    std::wstring sub_filename = lgvs::StringHelper::to_wstring (filename);
    std::wstring::size_type dot_pos = sub_filename.rfind (L'.');
    if (dot_pos != std::wstring::npos)
        sub_filename.erase (dot_pos, sub_filename.size () - dot_pos);
    sub_filename += L".srt";
    std::wstring::size_type slash_pos = sub_filename.find_last_of (L"\\/");
    if (slash_pos != std::wstring::npos)
        sub_filename.erase (0, slash_pos + 1);

    std::wstring sub_tmp_path;
    std::ifstream srt_stream;
    std::wstring cur_dir = sys_get_current_dir ();
    lgvs::WStringList movie_paths = read_movie_paths ();

    for (std::wstring::size_type i = 0; i < movie_paths.size (); ++i) {
        sub_tmp_path = cur_dir + L'\\' + movie_paths[i] + L'\\' + sub_filename;

        srt_stream.close ();
        srt_stream.clear ();
        srt_stream.open (sub_tmp_path.c_str ());

        if (srt_stream.is_open ())
            break;
    }

    if (srt_stream.is_open ())
        subtitles_ = lgvs::SrtParser::parse (srt_stream);
    else
        subtitles_.clear ();


    f_set_info_ = NULL;
    f_set_font_info_ = NULL;
    f_set_subs_ = NULL;
    f_show_sub_ = NULL;
    f_enable_subs_ = NULL;

    if (!subtitles_.empty ()) {
        d3d9_library_ = ::LoadLibraryW (L".\\d3d9.dll");

        if (d3d9_library_ != NULL) {
            const std::string msg = "\tSymbol nof found: ";
            const std::string dot = ".";

            f_set_info_ = reinterpret_cast<FP_LGS_SET_GLOBAL_INFO> (
                ::GetProcAddress (d3d9_library_, "lgs_set_global_info"));

            if (f_set_info_ == NULL)
                print_log (msg + "lgs_set_global_info" + dot);

            f_set_font_info_ = reinterpret_cast<FP_LGS_SET_GLOBAL_FONT_INFO> (
                ::GetProcAddress (d3d9_library_, "lgs_set_global_font_info"));

            if (f_set_info_ == NULL)
                print_log (msg + "lgs_set_global_font_info" + dot);

            f_set_subs_ = reinterpret_cast<FP_LGS_SET_SUBTITLES> (
                ::GetProcAddress (d3d9_library_, "lgs_set_subtitles"));

            if (f_set_info_ == NULL)
                print_log (msg + "lgs_set_subtitles" + dot);

            f_show_sub_ = reinterpret_cast<FP_LGS_SHOW_SUBTITLE> (
                ::GetProcAddress (d3d9_library_, "lgs_show_subtitle"));

            if (f_set_info_ == NULL)
                print_log (msg + "lgs_show_subtitle" + dot);

            f_enable_subs_ = reinterpret_cast<FP_LGS_ENABLE_SUBTITLES> (
                ::GetProcAddress (d3d9_library_, "lgs_enable_subtitles"));

            if (f_set_info_ == NULL)
                print_log (msg + "lgs_enable_subtitles" + dot);

            if (f_set_info_ != NULL &&
                f_set_font_info_ != NULL &&
                f_set_subs_ != NULL &&
                f_show_sub_ != NULL &&
                f_enable_subs_ != NULL)
            {
                host_ = host;

                font_filename_ = read_config_string (
                    "subs_font_filename", host);

                if (!font_filename_.empty ()) {
                    if (::AddFontResourceW (font_filename_.c_str ()) == 0) {
                        std::wstring msg = L"\tFailed to register a font: " +
                            font_filename_ + L".";
                        print_log (msg);
                        font_filename_.clear ();
                    }
                }

                read_config_values (host);

                f_set_info_ (g_info_);
                f_set_font_info_ (g_font_info_);
                f_set_subs_ (subtitles_);
                f_show_sub_ (-1);
                f_enable_subs_ (true);
            }
            else
                subtitles_.clear ();
        } else {
            subtitles_.clear ();
            print_log ("\tFailed to load a Direct3D 9 wrapper (.\\d3d9.dll).");
        }
    }

    if (!subtitles_.empty ())
        has_subtitles_ = true;
    else
        uninitialize ();
}

void LgVidSubs::uninitialize ()
{
    refresh_video = false;

    host_ = NULL;

    f_set_info_ = NULL;
    f_set_font_info_ = NULL;
    f_set_subs_ = NULL;
    f_show_sub_ = NULL;

    if (f_enable_subs_ != NULL) {
        f_enable_subs_ (false);
        f_enable_subs_ = NULL;
    }

    g_info_.reset ();
    g_font_info_.reset ();
    g_subs_.clear ();
    g_sub_index_ = -1;
    g_enable_subs_ = false;

    subtitles_.clear ();

    if (d3d9_library_ != NULL) {
        ::FreeLibrary (d3d9_library_);
        d3d9_library_ = NULL;
    }

    if (!font_filename_.empty ()) {
        ::RemoveFontResourceW (font_filename_.c_str ());
        font_filename_.clear ();
    }

    has_subtitles_ = false;
    last_sub_index_ = -1;
}

bool LgVidSubs::has_subtitles () const
{
    return has_subtitles_;
}

void LgVidSubs::print_log (const char* text)
{
    if (host_ == NULL)
        return;

    if (text == NULL)
        text = "";

    host_->LogPrint (text);
}

void LgVidSubs::print_log (const std::string& text)
{
    if (host_ == NULL)
        return;

    host_->LogPrint (text.c_str ());
}

void LgVidSubs::print_log (const std::wstring& text)
{
    print_log (lgvs::StringHelper::to_string (text));
}


} // namespace lgvs
