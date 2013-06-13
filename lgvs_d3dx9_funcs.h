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


#ifndef LGVS_D3DX9_FUNCS_H
#define LGVS_D3DX9_FUNCS_H


#include <d3dx9.h>


namespace lgvs {


class D3dx9Funcs {
public:
    D3dx9Funcs();
    ~D3dx9Funcs();

    bool initialize();
    void uninitialize();

    bool is_initialized() const;

    HRESULT create_font(
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
        LPD3DXFONT* ppFont) const;

    HRESULT create_sprite(
        LPDIRECT3DDEVICE9 pDevice,
        LPD3DXSPRITE* ppSprite) const;


private:
    typedef HRESULT (WINAPI* FP_D3DXCREATEFONT)(
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
        LPD3DXFONT* ppFont);

    typedef HRESULT (WINAPI* FP_D3DXCREATESPRITE)(
        LPDIRECT3DDEVICE9 pDevice,
        LPD3DXSPRITE* ppSprite);


    bool is_initialized_;
    HMODULE library_;
    FP_D3DXCREATEFONT create_font_;
    FP_D3DXCREATESPRITE create_sprite_;


    D3dx9Funcs(const D3dx9Funcs& that);
    D3dx9Funcs& operator=(const D3dx9Funcs& that);
}; // class D3dx9Funcs


} // namespace lgvs


#endif // LGVS_D3DX9_FUNCS_H
