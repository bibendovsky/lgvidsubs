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


#define CINTERFACE

#include "lgvs_detours.h"

#include <algorithm>
#include <bitset>
#include <new>

#include "lgvs_system.h"


namespace lgvs {


Detours::Detours() :
    is_initialized_(false),
    is_detoured_(false),
    d3d9_library_(nullptr),
    end_scene_detour_(),
    test_coop_level_detour_(),
    reset_detour_(),
    trampoline_cache_()
{
}

Detours::~Detours()
{
    uninitialize();
}

bool Detours::initialize(
    FP_D3DD9_END_SCENE fake_end_scene,
    FP_D3DD9_TEST_COOP_LEVEL fake_test_coop_level,
    FP_D3DD9_END_RESET fake_reset)
{
    uninitialize();

    error_string_.clear();

    if (fake_end_scene == nullptr) {
        error_string_ = L"Null \"EndScene\" fake function pointer.";
        return false;
    }

    if (fake_test_coop_level == nullptr) {
        error_string_ = L"Null \"TestCooperativeLevel\" fake function pointer.";
        return false;
    }

    if (fake_reset == nullptr) {
        error_string_ = L"Null \"Reset\" fake function pointer.";
        return false;
    }


    std::wstring d3d9_dll_path = System::combine_paths(
        System::get_system_dir(), L"d3d9.dll");

    d3d9_library_ = ::LoadLibraryW(d3d9_dll_path.c_str());

    if (d3d9_library_ == nullptr) {
        error_string_ = L"Failed to load a system Direct3D 9 main library.";
        return false;
    }


    bool is_succeed = true;

    ATOM dummy_class = 0;

    if (is_succeed) {
        WNDCLASSEXW wce;
        std::uninitialized_fill_n(
            reinterpret_cast<char*>(&wce), sizeof(wce), 0);
        wce.cbSize = sizeof(wce);
        wce.style = CS_CLASSDC;
        wce.lpfnWndProc = ::DefWindowProcW;
        wce.lpszClassName = L"LgVidSubsDummyClass";

        dummy_class = ::RegisterClassExW(&wce);

        if (dummy_class == 0) {
            is_succeed = false;
            error_string_ = L"Failed to register a dummy window class.";
        }
    }


    HWND dummy_window = nullptr;

    if (is_succeed) {
        dummy_window = ::CreateWindowW(
            reinterpret_cast <LPCWSTR>(dummy_class),
            L"LgVidSubs Dummy Window",
            WS_OVERLAPPED | WS_CAPTION | WS_CLIPCHILDREN,
            0, 0, 2, 2,
            ::GetDesktopWindow(),
            nullptr, nullptr, 0);

        if (dummy_window == nullptr) {
            is_succeed = false;
            error_string_ = L"Failed to create a dummy window.";
        }
    }


    typedef LPDIRECT3D9 (WINAPI* FP_DIRECT3DCREATE9)(UINT SDKVersion);
    FP_DIRECT3DCREATE9 d3d9_create = nullptr;

    if (is_succeed) {
        d3d9_create = reinterpret_cast<FP_DIRECT3DCREATE9>(
            ::GetProcAddress(d3d9_library_, "Direct3DCreate9"));

        if (d3d9_create == nullptr) {
            is_succeed = false;
            error_string_ = L"Missing symbol: Direct3DCreate9.";
        }
    }


    LPDIRECT3D9 d3d9 = nullptr;

    if (is_succeed) {
        d3d9 = d3d9_create(D3D_SDK_VERSION);

        if (d3d9 == nullptr) {
            is_succeed = false;
            error_string_ = L"Failed to create a Direct3D9 instance.";
        }
    }


    HRESULT d3d_result = D3D_OK;
    LPDIRECT3DDEVICE9 d3d_device9 = nullptr;

    if (is_succeed) {
        D3DPRESENT_PARAMETERS pp;
        std::uninitialized_fill_n(
            reinterpret_cast<char*>(&pp), sizeof(pp), 0);
        pp.SwapEffect = D3DSWAPEFFECT_FLIP;
        pp.Windowed = TRUE;

        d3d_result = IDirect3D9_CreateDevice(
            d3d9,
            D3DADAPTER_DEFAULT,
            D3DDEVTYPE_HAL,
            dummy_window,
            D3DCREATE_SOFTWARE_VERTEXPROCESSING,
            &pp,
            &d3d_device9);

        if (d3d_result != D3D_OK) {
            is_succeed = false;
            error_string_ = L"Failed to create a Direct3D 9 device instance.";
        }
    }

    if (is_succeed)
        is_succeed = trampoline_cache_.initialize(16, 16);

    if (is_succeed) {
        is_succeed &= end_scene_detour_.initialize(
            d3d_device9->lpVtbl->EndScene,
            fake_end_scene,
            7,
            &trampoline_cache_);

        is_succeed &= test_coop_level_detour_.initialize(
            d3d_device9->lpVtbl->TestCooperativeLevel,
            fake_test_coop_level,
            5,
            &trampoline_cache_);

        is_succeed &= reset_detour_.initialize(
            d3d_device9->lpVtbl->Reset,
            fake_reset,
            5,
            &trampoline_cache_);

        if (!is_succeed)
            error_string_ = L"Failed to detour Direct3D 9 APIs.";
    }

    if (d3d_device9 != nullptr)
        IDirect3DDevice9_Release(d3d_device9);

    if (d3d9 != nullptr)
        IDirect3D9_Release(d3d9);

    if (dummy_window != nullptr)
        ::DestroyWindow(dummy_window);

    if (dummy_class != 0)
        ::UnregisterClassW(reinterpret_cast<LPCWSTR>(dummy_class), nullptr);

    if (is_succeed)
        is_initialized_ = true;
    else
        uninitialize();

    return is_succeed;
}

void Detours::uninitialize()
{
    is_initialized_ = false;

    end_scene_detour_.uninitialize();
    test_coop_level_detour_.uninitialize();
    reset_detour_.uninitialize();

    trampoline_cache_.uninitialize();

    if (d3d9_library_ != nullptr) {
        ::FreeLibrary(d3d9_library_);
        d3d9_library_ = nullptr;
    }
}

bool Detours::is_initialized() const
{
    return is_initialized_;
}

const std::wstring& Detours::get_error_string() const
{
    return error_string_;
}

HRESULT STDMETHODCALLTYPE Detours::d3dd9_end_scene(
    LPDIRECT3DDEVICE9 self)
{
    return reinterpret_cast<FP_D3DD9_END_SCENE>(
        end_scene_detour_.get_trampoline())(self);
}

HRESULT STDMETHODCALLTYPE Detours::d3dd9_test_coop_level(
    LPDIRECT3DDEVICE9 self)
{
    return reinterpret_cast<FP_D3DD9_TEST_COOP_LEVEL>(
        test_coop_level_detour_.get_trampoline())(self);
}

HRESULT STDMETHODCALLTYPE Detours::d3dd9_reset(
    LPDIRECT3DDEVICE9 self,
    D3DPRESENT_PARAMETERS* presentation_parameters)
{
    return reinterpret_cast<FP_D3DD9_END_RESET>(
        reset_detour_.get_trampoline())(self, presentation_parameters);
}


} // namespace lgvs
