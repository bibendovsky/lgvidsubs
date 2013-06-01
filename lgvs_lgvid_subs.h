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


#ifndef LGVS_LGVID_SUBS_H
#define LGVS_LGVID_SUBS_H


#include <algorithm>
#include <fstream>
#include <sstream>

#include <windows.h>

#include "lgviddecoder.h"
#include "lgvs_dll_context.h"


namespace lgvs {


class LgVidSubs {
public:
    bool refresh_video;


    LgVidSubs ();
    ~LgVidSubs ();

    void initialize (ILGVideoDecoderHost* host, const char* filename);
    void uninitialize ();

    bool has_subtitles () const;

    bool check_subtitle (double pts);


private:
    ILGVideoDecoderHost* host_;

    FP_LGS_SET_GLOBAL_INFO f_set_info_;
    FP_LGS_SET_GLOBAL_FONT_INFO f_set_font_info_;
    FP_LGS_SET_SUBTITLES f_set_subs_;
    FP_LGS_SHOW_SUBTITLE f_show_sub_;
    FP_LGS_ENABLE_SUBTITLES f_enable_subs_;

    static const std::wstring BLANK_WSTRING;
    static const std::wstring DEFAULT_FONT_FAMILY;

    std::wistringstream ss_buffer_;

    GlobalInfo g_info_;
    GlobalFontInfo g_font_info_;
    lgvs::SubtitleList g_subs_;
    int g_sub_index_;
    bool g_enable_subs_;

    lgvs::SubtitleList subtitles_;
    HMODULE d3d9_library_;
    std::wstring font_filename_;
    bool has_subtitles_;
    int last_sub_index_;

    std::wstring read_config_string (const char* key_name,
        ILGVideoDecoderHost* host,
        const std::wstring& default_value = BLANK_WSTRING);

    float read_config_float_value (const char* key_name,
        ILGVideoDecoderHost* host, float default_value);

    float read_config_percents_value (const char* key_name,
        ILGVideoDecoderHost* host, float default_value,
        bool default_is_percents, bool& is_percents);

    unsigned read_color_value (const char* key_name, ILGVideoDecoderHost* host,
        unsigned default_value);

    void read_config_values (ILGVideoDecoderHost* host);

    std::wstring sys_get_current_dir ();
    lgvs::WStringList read_movie_paths ();

    void print_log (const char* text = NULL);
    void print_log (const std::string& text);
    void print_log (const std::wstring& text);


    LgVidSubs (const LgVidSubs& that);
    LgVidSubs& operator = (const LgVidSubs& that);
}; // class LgVidSubs


} // namespace lgvs


#endif // LGVS_LGVID_SUBS_H
