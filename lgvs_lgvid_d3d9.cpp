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


#include <memory>
#include <string>
#include <vector>

#include <d3dx9.h>

#include "lgvs_dll_context.h"
#include "lgvs_subtitle.h"
#include "lgvs_fake_d3d9.h"


namespace lgvs {


GlobalInfo g_info;
GlobalFontInfo g_font_info;
SubtitleList g_subs;
int g_sub_index;
bool g_enable_subs;
int g_old_sub_index;


void lgs_set_global_info (const lgvs::GlobalInfo& info)
{
    g_info = info;
}

void lgs_set_global_font_info (const lgvs::GlobalFontInfo& font_info)
{
    g_font_info = font_info;
}

void lgs_set_subtitles (const lgvs::SubtitleList& subs)
{
    g_subs = subs;
}

void lgs_show_subtitle (int index)
{
    g_sub_index = index;
}

void lgs_enable_subtitles (bool value)
{
    g_enable_subs = value;

    if (!value) {
        g_info.reset ();
        g_font_info.reset ();
        g_subs.clear ();
        g_sub_index = -1;
        g_old_sub_index = -1;
    }
}


} // namespace lgvs


typedef IDirect3D9* (WINAPI* FP_DIRECT3DCREATE9) (UINT SDKVersion);


IDirect3D9* WINAPI Direct3DCreate9 (UINT SDKVersion)
{
    wchar_t name_buffer[MAX_PATH];

    ::GetSystemDirectoryW (name_buffer, MAX_PATH);

    if (::GetLastError () != ERROR_SUCCESS)
        return NULL;

    std::wstring system_dir = name_buffer;
    std::wstring d3d9_library_name = system_dir + L"\\d3d9.dll";
    std::wstring d3dx9_library_name = system_dir + L"\\d3dx9_43.dll";

    IDirect3D9* fake_d3d9 = NULL;
    IDirect3D9* real_d3d9 = NULL;
    HMODULE d3d9_module = ::LoadLibraryW (d3d9_library_name.c_str ());

    if (d3d9_module != NULL) {
        FP_DIRECT3DCREATE9 d3dCreate9Func = reinterpret_cast<FP_DIRECT3DCREATE9> (
            ::GetProcAddress (d3d9_module, "Direct3DCreate9"));

        if (d3dCreate9Func != NULL) {
            real_d3d9 = d3dCreate9Func (SDKVersion);

            if (real_d3d9 != NULL) {
                fake_d3d9 = new FakeD3d9 (
                    d3d9_module, real_d3d9, d3dx9_library_name.c_str ());
            }
        }
    }

    if (fake_d3d9 == NULL) {
        if (real_d3d9 != NULL)
            real_d3d9->Release ();

        if (d3d9_module != NULL)
            ::FreeLibrary (d3d9_module);
    }

    return fake_d3d9;
}

BOOL WINAPI DllMain (HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        lgvs::g_info.reset ();
        lgvs::g_font_info.reset ();
        lgvs::g_subs.clear ();
        lgvs::g_sub_index = -1;
        lgvs::g_enable_subs = false;
        lgvs::g_old_sub_index = -1;
        break;
    }

    return TRUE;
}
