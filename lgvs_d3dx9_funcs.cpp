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


#include "lgvs_d3dx9_funcs.h"

#include <string>

#include "lgvs_system.h"


namespace lgvs {


D3dx9Funcs::D3dx9Funcs() :
    is_initialized_(false),
    library_(NULL),
    create_font_(NULL),
    create_sprite_(NULL)
{
    if (library_ == NULL)
        return;

    create_font_ = reinterpret_cast<FP_D3DXCREATEFONT>(
        ::GetProcAddress (library_, "D3DXCreateFontW"));

    create_sprite_ = reinterpret_cast<FP_D3DXCREATESPRITE>(
        ::GetProcAddress (library_, "D3DXCreateSprite"));

    if (create_font_ != NULL &&
        create_sprite_ != NULL)
    {
        is_initialized_ = true;
    } else {
        create_font_ = NULL;
        create_sprite_ = NULL;

        ::FreeLibrary (library_);
        library_ = NULL;
    }
}

D3dx9Funcs::~D3dx9Funcs()
{
    if (library_ != NULL)
        ::FreeLibrary (library_);
}

bool D3dx9Funcs::initialize()
{
    uninitialize();

    std::wstring dll_file_path;

    dll_file_path = System::combine_paths(
        System::get_system_dir(), L"d3dx9_43.dll");

    library_ = ::LoadLibraryW(dll_file_path.c_str());

    if (library_ == NULL)
        return false;

    create_font_ = reinterpret_cast<FP_D3DXCREATEFONT>(
        ::GetProcAddress(library_, "D3DXCreateFontW"));

    create_sprite_ = reinterpret_cast<FP_D3DXCREATESPRITE>(
        ::GetProcAddress(library_, "D3DXCreateSprite"));

    if (create_font_ != NULL && create_sprite_ != NULL)
        is_initialized_ = true;
    else
        uninitialize();

    return is_initialized_;
}

void D3dx9Funcs::uninitialize()
{
    is_initialized_ = false;

    if (library_ != NULL) {
        ::FreeLibrary(library_);
        library_ = NULL;
    }

    create_font_ = NULL;
    create_sprite_ = NULL;
}

bool D3dx9Funcs::is_initialized() const
{
    return is_initialized_;
}

HRESULT D3dx9Funcs::create_font(
    LPDIRECT3DDEVICE9 pDevice,
    INT Height,
    UINT Width,
    UINT Weight,
    UINT MipLevels,
    BOOL Italic,
    DWORD CharSet,
    DWORD OutputPrecision,
    DWORD Quality,
    DWORD PitchAndFamily,
    LPCWSTR pFacename,
    LPD3DXFONT* ppFont) const
{
    return create_font_(
        pDevice,
        Height,
        Width,
        Weight,
        MipLevels,
        Italic,
        CharSet,
        OutputPrecision,
        Quality,
        PitchAndFamily,
        pFacename,
        ppFont);
}

HRESULT D3dx9Funcs::create_sprite(
    LPDIRECT3DDEVICE9 pDevice,
    LPD3DXSPRITE* ppSprite) const
{
    return create_sprite_(pDevice, ppSprite);
}


} // namespace lgvs
