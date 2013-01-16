#ifndef D3DX9_FUNCS_H
#define D3DX9_FUNCS_H


#include <d3dx9.h>
#include <tchar.h>


class D3dX9Funcs {
public:
    D3dX9Funcs (LPCTSTR library_name);

    ~D3dX9Funcs ();


    bool is_initialized () const;

    HRESULT WINAPI create_font (
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
        LPCTSTR pFacename,
        LPD3DXFONT* ppFont) const;

    HRESULT WINAPI create_sprite (
        LPDIRECT3DDEVICE9 pDevice,
        LPD3DXSPRITE* ppSprite) const;


private:
    typedef HRESULT (WINAPI* FP_D3DXCREATEFONT) (
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
        LPCTSTR pFacename,
        LPD3DXFONT* ppFont);

    typedef HRESULT (WINAPI* FP_D3DXCREATESPRITE) (
        LPDIRECT3DDEVICE9 pDevice,
        LPD3DXSPRITE* ppSprite);


    bool is_initialized_;
    HMODULE library_;
    FP_D3DXCREATEFONT create_font_;
    FP_D3DXCREATESPRITE create_sprite_;


    D3dX9Funcs (const D3dX9Funcs& that);

    D3dX9Funcs& operator = (const D3dX9Funcs& that);
}; // class D3dX9Funcs


#endif // D3DX9_FUNCS_H
