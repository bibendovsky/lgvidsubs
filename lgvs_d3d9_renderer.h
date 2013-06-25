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

#ifndef LGVS_D3D9_RENDERER_H
#define LGVS_D3D9_RENDERER_H


#include <vector>

#include "lgvs_d3dx9_funcs.h"
#include "lgvs_detours.h"


namespace lgvs {


class D3d9Renderer {
public:
    static bool initialize();
    static void uninitialize();
    static bool is_initialized();
    static const std::wstring& get_error_string();


private:
    static bool is_initialized_;
    static std::wstring error_string_;
    static LPDIRECT3DDEVICE9 device_;
    static bool is_device_lost_;
    static D3dx9Funcs x_funcs_;
    static D3DVIEWPORT9 view_port_;
    static LPD3DXFONT font_;
    static LPD3DXSPRITE sprite_;
    static int font_height_;
    static int font_weight_;
    static int text_height_;
    static int space_after_;
    static int shadow_offset_x_;
    static int shadow_offset_y_;
    static int last_sub_index_;
    static std::vector<SIZE> lines_sizes_;
    static Detours detours_;

    static void draw_subtitle();
    static void calculate_props();
    static void measure_text();

    static HRESULT STDMETHODCALLTYPE fake_end_scene(
        LPDIRECT3DDEVICE9 self);
    static HRESULT STDMETHODCALLTYPE fake_test_cooperative_level(
        LPDIRECT3DDEVICE9 self);
    static HRESULT STDMETHODCALLTYPE fake_reset(
        LPDIRECT3DDEVICE9 self,
        D3DPRESENT_PARAMETERS* presentation_parameters);

    D3d9Renderer();
    D3d9Renderer(const D3d9Renderer& that);
    ~D3d9Renderer();
    D3d9Renderer& operator=(const D3d9Renderer& that);
}; // class D3d9Renderer


} // namespace lgvs


#endif // LGVS_D3D9_RENDERER_H
