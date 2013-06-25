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

#include "lgvs_config.h"
#include "lgvs_d3d9_renderer.h"
#include "lgvs_types.h"


namespace lgvs {


class LgVidSubs {
public:
    bool refresh_video;
    bool refresh_subtitle;

    LgVidSubs();
    ~LgVidSubs();

    void initialize(ILGVideoDecoderHost* host, const char* file_path);
    void uninitialize();

    bool has_subtitles() const;

    bool check_subtitle(double pts);


private:
    ILGVideoDecoderHost* host_;
    Config config_;

    static const std::wstring BLANK_WSTRING;
    static const std::wstring DEFAULT_FONT_FAMILY;

    std::wstring font_filename_;
    bool has_subtitles_;

    void read_config_values();

    WStringList read_movie_paths();

    void print_log(const char* text = nullptr);
    void print_log(const std::string& text);
    void print_log(const std::wstring& text);


    LgVidSubs(const LgVidSubs& that);
    LgVidSubs& operator=(const LgVidSubs& that);
}; // class LgVidSubs


} // namespace lgvs


#endif // LGVS_LGVID_SUBS_H
