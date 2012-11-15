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
    LPCTSTR libraryName) :
        mIsInitialized (false),
        mLibrary (::LoadLibrary (libraryName)),
        mCreateFont (0),
        mCreateSprite (0)
{
    if (mLibrary == 0)
        return;

    mCreateFont = reinterpret_cast<FP_D3DXCREATEFONT> (
        ::GetProcAddress (mLibrary, D3DX_CREATE_FONT_FUNC_NAME));

    mCreateSprite = reinterpret_cast<FP_D3DXCREATESPRITE> (
        ::GetProcAddress (mLibrary, D3DX_CREATE_SPRITE_FUNC_NAME));

    if ((mCreateFont != 0) &&
        (mCreateSprite != 0))
    {
        mIsInitialized = true;
    } else {
        mCreateFont = 0;
        mCreateSprite = 0;

        ::FreeLibrary (mLibrary);
        mLibrary = 0;
    }
}

D3dX9Funcs::~D3dX9Funcs ()
{
    if (mLibrary != 0)
        ::FreeLibrary (mLibrary);
}

bool D3dX9Funcs::isInitialized () const
{
    return mIsInitialized;
}

HRESULT WINAPI D3dX9Funcs::createFont (
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
    if ((!mIsInitialized) || (ppFont == 0)) {
        if (ppFont != 0)
            *ppFont = 0;

        return D3DERR_INVALIDCALL;
    }

    return mCreateFont (pDevice, Height, Width, Weight, MipLevels, Italic,
        CharSet, OutputPrecision, Quality, PitchAndFamily, pFacename, ppFont);
}

HRESULT WINAPI D3dX9Funcs::createSprite (
    LPDIRECT3DDEVICE9 pDevice,
    LPD3DXSPRITE* ppSprite) const
{
    if ((!mIsInitialized) || (ppSprite == 0)) {
        if (ppSprite != 0)
            *ppSprite = 0;

        return D3DERR_INVALIDCALL;
    }

    return mCreateSprite (pDevice, ppSprite);
}
