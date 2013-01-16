#include "d3dx9_funcs.h"


namespace {


const char* const D3DX_CREATE_FONT_FUNC_NAME =
#ifdef _UNICODE
    "D3DXCreateFontW"
#else
    "D3DXCreateFontA"
#endif // _UNICODE
    ;

const char* const D3DX_CREATE_SPRITE_FUNC_NAME = "D3DXCreateSprite";


} // namespace


D3dX9Funcs::D3dX9Funcs (
    LPCTSTR library_name) :
        is_initialized_ (false),
        library_ (::LoadLibrary (library_name)),
        create_font_ (0),
        create_sprite_ (0)
{
    if (library_ == 0)
        return;

    create_font_ = reinterpret_cast<FP_D3DXCREATEFONT> (
        ::GetProcAddress (library_, D3DX_CREATE_FONT_FUNC_NAME));

    create_sprite_ = reinterpret_cast<FP_D3DXCREATESPRITE> (
        ::GetProcAddress (library_, D3DX_CREATE_SPRITE_FUNC_NAME));

    if ((create_font_ != 0) &&
        (create_sprite_ != 0))
    {
        is_initialized_ = true;
    } else {
        create_font_ = 0;
        create_sprite_ = 0;

        ::FreeLibrary (library_);
        library_ = 0;
    }
}

D3dX9Funcs::~D3dX9Funcs ()
{
    if (library_ != 0)
        ::FreeLibrary (library_);
}

bool D3dX9Funcs::is_initialized () const
{
    return is_initialized_;
}

HRESULT WINAPI D3dX9Funcs::create_font (
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
    LPD3DXFONT* ppFont) const
{
    if ((!is_initialized_) || (ppFont == 0)) {
        if (ppFont != 0)
            *ppFont = 0;

        return D3DERR_INVALIDCALL;
    }

    return create_font_ (pDevice, Height, Width, Weight, MipLevels, Italic,
        CharSet, OutputPrecision, Quality, PitchAndFamily, pFacename, ppFont);
}

HRESULT WINAPI D3dX9Funcs::create_sprite (
    LPDIRECT3DDEVICE9 pDevice,
    LPD3DXSPRITE* ppSprite) const
{
    if ((!is_initialized_) || (ppSprite == 0)) {
        if (ppSprite != 0)
            *ppSprite = 0;

        return D3DERR_INVALIDCALL;
    }

    return create_sprite_ (pDevice, ppSprite);
}
