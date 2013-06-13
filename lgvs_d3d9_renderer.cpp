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


#include "lgvs_d3d9_renderer.h"

#include "lgvs_com_helper.h"
#include "lgvs_globals.h"


namespace lgvs {


bool D3d9Renderer::is_initialized_;
std::wstring D3d9Renderer::error_string_;
LPDIRECT3DDEVICE9 D3d9Renderer::device_;
bool D3d9Renderer::is_device_lost_;
D3dx9Funcs D3d9Renderer::x_funcs_;
D3DVIEWPORT9 D3d9Renderer::view_port_;
LPD3DXFONT D3d9Renderer::font_;
LPD3DXSPRITE D3d9Renderer::sprite_;
int D3d9Renderer::font_height_;
int D3d9Renderer::font_weight_;
int D3d9Renderer::text_height_;
int D3d9Renderer::space_after_;
int D3d9Renderer::shadow_offset_x_;
int D3d9Renderer::shadow_offset_y_;
int D3d9Renderer::last_sub_index_;
std::vector<SIZE> D3d9Renderer::lines_sizes_;
Detours D3d9Renderer::detours_;


// (static)
bool D3d9Renderer::initialize()
{
    uninitialize();

    error_string_.clear();

    bool is_succeed = true;

    if (is_succeed)
        is_succeed = x_funcs_.initialize();

    if (is_succeed) {
        is_succeed = detours_.initialize(
            fake_end_scene,
            fake_test_cooperative_level,
            fake_reset);
    }

    if (is_succeed)
        is_initialized_ = true;
    else {
        uninitialize();
        error_string_ = detours_.get_error_string();
    }

    return is_succeed;
}

// (static)
void D3d9Renderer::uninitialize()
{
    is_initialized_ = false;
    device_ = NULL;
    is_device_lost_ = false;

    font_height_ = 0;
    font_weight_ = 0;
    text_height_ = 0;
    space_after_ = 0;
    shadow_offset_x_ = 0;
    shadow_offset_y_ = 0;
    last_sub_index_ = -1;
    lines_sizes_.clear();

    if (font_ != NULL) {
        ComHelper::release_and_null(font_);
        font_ = NULL;
    }

    if (sprite_ != NULL) {
        ComHelper::release_and_null(sprite_);
        sprite_ = NULL;
    }

    x_funcs_.uninitialize();
    detours_.uninitialize();
}

// (static)
bool D3d9Renderer::is_initialized()
{
    return is_initialized_;
}

// (static)
const std::wstring& D3d9Renderer::get_error_string()
{
    return error_string_;
}

// (static)
void D3d9Renderer::calculate_props()
{
    font_height_ = 0;
    text_height_ = 0;
    space_after_ = 0;

    if (!is_initialized_)
        return;

    if (device_ == NULL)
        return;

    if (!Globals::enable_subs)
        return;


    HRESULT d3d_result;

    d3d_result = device_->GetViewport(&view_port_);

    if (d3d_result != D3D_OK)
        return;


    float font_height_f = Globals::font_size;

    if (Globals::font_size_in_percents)
        font_height_f *= view_port_.Height * 0.01F;

    font_height_ = static_cast<int>(font_height_f);
    font_weight_ = static_cast<int>(Globals::font_weight);

    float space_after_f = Globals::space_after;

    if (Globals::space_after_in_percents)
        space_after_f *= view_port_.Height * 0.01F;

    if (space_after_f < 0.0F)
        space_after_f = 0.0F;

    space_after_ = static_cast<int>(space_after_f);


    float shadow_offset_xf = Globals::shadow_offset_x;

    if (Globals::shadow_offset_x_in_percents)
        shadow_offset_xf *= view_port_.Height * 0.01F;

    shadow_offset_x_ = static_cast<int>(shadow_offset_xf);


    float shadow_offset_y_f = Globals::shadow_offset_y;

    if (Globals::shadow_offset_y_in_percents)
        shadow_offset_y_f *= view_port_.Height * 0.01F;

    shadow_offset_y_ = static_cast<int>(shadow_offset_y_f);
}

// (static)
void D3d9Renderer::measure_text ()
{
    text_height_ = 0;

    if (!is_initialized_)
        return;

    if (Globals::sub_index < 0)
        return;

    if (Globals::subs.empty())
        return;

    RECT rect;
    HRESULT d3d_result;

    const Subtitle& subtitle =
        Globals::subs[Globals::sub_index];
    const WStringList& lines = subtitle.lines;
    lines_sizes_.resize(lines.size());

    for (size_t i = 0; i < lines.size(); ++i) {
        const std::wstring& line = lines[i];
        size_t line_length = line.size();
        SIZE& line_size = lines_sizes_[i];

        if (line_length > 0) {
            rect.left = 0;
            rect.top = 0;
            rect.right = 0;
            rect.bottom = 0;

            d3d_result = font_->PreloadTextW(line.c_str(), line_length);

            INT text_result = font_->DrawTextW(
                sprite_, // sprite
                line.c_str(), // text
                line_length, // count,
                &rect, // rectangle
                DT_NOCLIP | DT_LEFT | DT_TOP | DT_SINGLELINE | DT_CALCRECT, // format
                0 // color
            );

            if (text_result == 0)
                text_result = font_height_ - 1;

            line_size.cy = text_result + 1;
            line_size.cx = rect.right + 1;

            text_height_ += line_size.cy;
        } else {
            text_height_ += font_height_;
            line_size.cx = 0;
            line_size.cy = 0;
        }
    }
}

// (static)
void D3d9Renderer::draw_subtitle()
{
    if (!is_initialized_)
        return;

    if (device_ == NULL)
        return;

    if (is_device_lost_)
        return;

    if (Globals::sub_index < 0)
        return;

    const Subtitle& subtitle =
        Globals::subs[Globals::sub_index];

    size_t line_count = subtitle.lines.size();

    if (line_count == 0)
        return;


    HRESULT d3d_result = D3D_OK;

    if (sprite_ == NULL)
        d3d_result = x_funcs_.create_sprite(device_, &sprite_);

    if (font_ == NULL) {
        calculate_props();

        if (font_height_ > 0) {
            d3d_result = x_funcs_.create_font(
                device_,
                font_height_, // height
                0, // width
                font_weight_, // weight
                0, // mipmap levels
                FALSE, // italic?
                DEFAULT_CHARSET, // charset
                OUT_TT_PRECIS, // precision
                DEFAULT_QUALITY, // quality
                FF_DONTCARE | DEFAULT_PITCH, // pitch and family
                Globals::font_family.c_str(), // typeface name
                &font_ // result
            );
        }
    }

    if (font_ == NULL)
        return;


    if (last_sub_index_ != Globals::sub_index) {
        measure_text();
        last_sub_index_ = Globals::sub_index;
    }

    if (sprite_ != NULL)
        d3d_result = sprite_->Begin(D3DXSPRITE_ALPHABLEND);

    INT text_result = 0;

    RECT rect;


    int base_x = view_port_.X;
    int y = view_port_.Y + view_port_.Height - text_height_ - space_after_;
    const WStringList& lines = subtitle.lines;

    for (size_t i = 0; i < line_count; ++i) {
        const std::wstring& line = lines[i];
        size_t line_length = line.size();
        const SIZE& line_size = lines_sizes_[i];

        if ((line_length > 0) && ((line_size.cx > 0) && (line_size.cy > 0))) {
            rect.left = base_x + (static_cast<LONG>(view_port_.Width) - line_size.cx) / 2;
            rect.top = y;
            rect.right = rect.left + line_size.cx;
            rect.bottom = rect.top + line_size.cy;

            if (shadow_offset_x_ != 0 || shadow_offset_y_ != 0) {
                rect.left += shadow_offset_x_;
                rect.top += shadow_offset_y_;

                text_result = font_->DrawTextW(
                    sprite_, // sprite
                    line.c_str(), // text
                    line_length, // count,
                    &rect, // rectangle
                    DT_NOCLIP | DT_LEFT | DT_TOP | DT_SINGLELINE, // format
                    Globals::shadow_color // color
                );

                rect.left -= shadow_offset_x_;
                rect.top -= shadow_offset_y_;
            }

            text_result = font_->DrawTextW(
                sprite_, // sprite
                line.c_str(), // text
                line_length, // count,
                &rect, // rectangle
                DT_NOCLIP | DT_LEFT | DT_TOP | DT_SINGLELINE, // format
                Globals::font_color // color
            );

            y += line_size.cy;
        } else
            y += font_height_;
    }

    if (sprite_ != NULL)
        d3d_result = sprite_->End();
}

// (static)
HRESULT STDMETHODCALLTYPE D3d9Renderer::fake_end_scene(
    LPDIRECT3DDEVICE9 zis)
{
    if (device_ == NULL)
        device_ = zis;

    draw_subtitle();

    return detours_.d3dd9_end_scene(zis);
}

// (static)
HRESULT STDMETHODCALLTYPE D3d9Renderer::fake_test_cooperative_level(
    LPDIRECT3DDEVICE9 zis)
{
    if (device_ == NULL)
        device_ = zis;

    HRESULT d3d_result = detours_.d3dd9_test_cooperative_level(zis);

    switch (d3d_result) {
    case D3D_OK:
    case D3DERR_DEVICENOTRESET:
        is_device_lost_ = false;
        break;

    default:
        is_device_lost_ = true;
        break;
    }

    if (is_device_lost_) {
        if (font_ != NULL)
            font_->OnLostDevice();

        if (sprite_ != NULL)
            sprite_->OnLostDevice();
    }

    return d3d_result;
}

// (static)
HRESULT STDMETHODCALLTYPE D3d9Renderer::fake_reset(
    LPDIRECT3DDEVICE9 zis,
    D3DPRESENT_PARAMETERS* presentation_parameters)
{
    if (device_ == NULL)
        device_ = zis;

    if (font_ != NULL)
        font_->OnResetDevice();

    if (sprite_ != NULL)
        sprite_->OnResetDevice();

    return detours_.d3dd9_reset(zis, presentation_parameters);
}


} // namespace lgvs
