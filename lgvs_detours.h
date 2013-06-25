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


#ifndef LGVS_DETOURS_H
#define LGVS_DETOURS_H


#include <string>

#include <d3d9.h>

#include "lgvs_detour.h"


namespace lgvs {


class Detours {
public:
    typedef HRESULT (STDMETHODCALLTYPE* FP_D3DD9_END_SCENE)
        (LPDIRECT3DDEVICE9 self);

    typedef HRESULT (STDMETHODCALLTYPE* FP_D3DD9_TEST_COOP_LEVEL)
        (LPDIRECT3DDEVICE9 self);

    typedef HRESULT (STDMETHODCALLTYPE* FP_D3DD9_END_RESET)
        (LPDIRECT3DDEVICE9 self,
        D3DPRESENT_PARAMETERS* presentation_parameters);


    Detours();
    ~Detours();

    bool initialize(
        FP_D3DD9_END_SCENE fake_end_scene,
        FP_D3DD9_TEST_COOP_LEVEL fake_test_coop_level,
        FP_D3DD9_END_RESET fake_reset);

    void uninitialize();

    bool is_initialized() const;

    const std::wstring& get_error_string() const;

    HRESULT STDMETHODCALLTYPE d3dd9_end_scene(
        LPDIRECT3DDEVICE9 zis);

    HRESULT STDMETHODCALLTYPE d3dd9_test_coop_level(
        LPDIRECT3DDEVICE9 zis);

    HRESULT STDMETHODCALLTYPE d3dd9_reset(
        LPDIRECT3DDEVICE9 zis,
        D3DPRESENT_PARAMETERS* presentation_parameters);


private:
    bool is_initialized_;
    std::wstring error_string_;
    bool is_detoured_;
    HMODULE d3d9_library_;
    Detour end_scene_detour_;
    Detour test_coop_level_detour_;
    Detour reset_detour_;
    TrampolineCache trampoline_cache_;

    Detours(const Detours& that);
    Detours& operator=(const Detours& that);
}; // class Detours


} // namespace lgvs



#endif // LGVS_DETOURS_H
