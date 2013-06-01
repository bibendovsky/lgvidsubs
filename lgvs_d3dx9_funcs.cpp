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


D3dx9Funcs::D3dx9Funcs (LPCWSTR library_name) :
    is_initialized_ (false),
    library_ (::LoadLibrary (library_name)),
    create_font_ (NULL),
    create_sprite_ (NULL)
{
    if (library_ == NULL)
        return;

    create_font_ = reinterpret_cast<FP_D3DXCREATEFONT> (
        ::GetProcAddress (library_, "D3DXCreateFontW"));

    create_sprite_ = reinterpret_cast<FP_D3DXCREATESPRITE> (
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

D3dx9Funcs::~D3dx9Funcs ()
{
    if (library_ != NULL)
        ::FreeLibrary (library_);
}

bool D3dx9Funcs::is_initialized () const
{
    return is_initialized_;
}

HRESULT WINAPI D3dx9Funcs::create_font (
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
    if ((!is_initialized_) || ppFont == NULL) {
        if (ppFont != NULL)
            *ppFont = NULL;

        return D3DERR_INVALIDCALL;
    }

    return create_font_ (pDevice, Height, Width, Weight, MipLevels, Italic,
        CharSet, OutputPrecision, Quality, PitchAndFamily, pFacename, ppFont);
}

HRESULT WINAPI D3dx9Funcs::create_sprite (
    LPDIRECT3DDEVICE9 pDevice,
    LPD3DXSPRITE* ppSprite) const
{
    if ((!is_initialized_) || ppSprite == NULL) {
        if (ppSprite != NULL)
            *ppSprite = NULL;

        return D3DERR_INVALIDCALL;
    }

    return create_sprite_ (pDevice, ppSprite);
}
