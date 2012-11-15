#include "bbi_com_helper.h"
#include "bbi_dll_context.h"
#include "bbi_math.h"
#include "fake_direct3d_device9.h"


extern bbi::DllContext LgVidContext;


FakeDirect3dDevice9::FakeDirect3dDevice9 (
    IDirect3DDevice9* realDevice,
    D3dX9Funcs* d3dX9Funcs) :
        mRealDevice (realDevice),
        mIsDeviceLost (false),
        mD3dX9Funcs (d3dX9Funcs),
        mFont (0),
        mSprite (0),
        mOldSubIndex (-1),
        mFontHeight (0),
        mFontWeight (0),
        mTextHeight (0),
        mSpaceAfter (0),
        mShadowOffsetX (0),
        mShadowOffsetY (0)
{
}

FakeDirect3dDevice9::~FakeDirect3dDevice9 ()
{
    bbi::ComHelper::releaseAndNull (mFont);
    bbi::ComHelper::releaseAndNull (mSprite);
}


HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::QueryInterface (
    REFIID riid,
    void** ppvObj)
{
    if (ppvObj == 0)
        return E_POINTER;

    if ((riid == IID_IUnknown) ||
        (riid == IID_IDirect3DDevice9))
    {
        *ppvObj = this;
        return D3D_OK;
    }

    return mRealDevice->QueryInterface (riid, ppvObj);
}

ULONG STDMETHODCALLTYPE FakeDirect3dDevice9::AddRef ()
{
    return mRealDevice->AddRef ();
}

ULONG STDMETHODCALLTYPE FakeDirect3dDevice9::Release ()
{
    ULONG result = mRealDevice->Release ();

    if (result == 0)
        delete this;

    return result;
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::TestCooperativeLevel ()
{
    HRESULT result = mRealDevice->TestCooperativeLevel ();

    mIsDeviceLost = (result != D3D_OK);

    if (mIsDeviceLost) {
        HRESULT d3dResult = D3D_OK;

        if (mSprite != 0)
            d3dResult = mSprite->OnLostDevice ();

        if (mFont != 0)
            d3dResult = mFont->OnLostDevice ();
    }

    return result;
}

UINT STDMETHODCALLTYPE FakeDirect3dDevice9::GetAvailableTextureMem ()
{
    return mRealDevice->GetAvailableTextureMem ();
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::EvictManagedResources ()
{
    return mRealDevice->EvictManagedResources ();
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetDirect3D (
    IDirect3D9** ppD3D9)
{
    return mRealDevice->GetDirect3D (ppD3D9);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetDeviceCaps (
    D3DCAPS9* pCaps)
{
    return mRealDevice->GetDeviceCaps (pCaps);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetDisplayMode (
    UINT iSwapChain,
    D3DDISPLAYMODE* pMode)
{
    return mRealDevice->GetDisplayMode (iSwapChain, pMode);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetCreationParameters (
    D3DDEVICE_CREATION_PARAMETERS *pParameters)
{
    return mRealDevice->GetCreationParameters (pParameters);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetCursorProperties (
    UINT XHotSpot,
    UINT YHotSpot,
    IDirect3DSurface9* pCursorBitmap)
{
    return mRealDevice->SetCursorProperties (XHotSpot, YHotSpot, pCursorBitmap);
}

void STDMETHODCALLTYPE FakeDirect3dDevice9::SetCursorPosition (
    int X,
    int Y,
    DWORD Flags)
{
    return mRealDevice->SetCursorPosition (X, Y, Flags);
}

BOOL STDMETHODCALLTYPE FakeDirect3dDevice9::ShowCursor (
    BOOL bShow)
{
    return mRealDevice->ShowCursor (bShow);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::CreateAdditionalSwapChain (
    D3DPRESENT_PARAMETERS* pPresentationParameters,
    IDirect3DSwapChain9** pSwapChain)
{
    return mRealDevice->CreateAdditionalSwapChain (pPresentationParameters, pSwapChain);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetSwapChain (
    UINT iSwapChain,
    IDirect3DSwapChain9** pSwapChain)
{
    return mRealDevice->GetSwapChain (iSwapChain, pSwapChain);
}

UINT STDMETHODCALLTYPE FakeDirect3dDevice9::GetNumberOfSwapChains ( )
{
    return mRealDevice->GetNumberOfSwapChains ();
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::Reset (
    D3DPRESENT_PARAMETERS* pPresentationParameters)
{
    HRESULT d3dResult = D3D_OK;

    if (LgVidContext.showSubs) {
        if (mSprite != 0)
            d3dResult = mSprite->OnLostDevice ();

        if (mFont != 0)
            d3dResult = mFont->OnLostDevice ();
    }

    HRESULT result = mRealDevice->Reset (pPresentationParameters);

    if (LgVidContext.showSubs)
        measureText ();

    return result;
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::Present (
    CONST RECT* pSourceRect,
    CONST RECT* pDestRect,
    HWND hDestWindowOverride,
    CONST RGNDATA* pDirtyRegion)
{
    return mRealDevice->Present (
        pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetBackBuffer (
    UINT iSwapChain,
    UINT iBackBuffer,
    D3DBACKBUFFER_TYPE Type,
    IDirect3DSurface9** ppBackBuffer)
{
    return mRealDevice->GetBackBuffer (iSwapChain, iBackBuffer, Type, ppBackBuffer);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetRasterStatus (
    UINT iSwapChain,
    D3DRASTER_STATUS* pRasterStatus)
{
    return mRealDevice->GetRasterStatus (iSwapChain, pRasterStatus);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetDialogBoxMode (
    BOOL bEnableDialogs)
{
    return mRealDevice->SetDialogBoxMode (bEnableDialogs);
}

void STDMETHODCALLTYPE FakeDirect3dDevice9::SetGammaRamp (
    UINT iSwapChain,
    DWORD Flags,
    CONST D3DGAMMARAMP* pRamp)
{
    return mRealDevice->SetGammaRamp (iSwapChain, Flags, pRamp);
}

void STDMETHODCALLTYPE FakeDirect3dDevice9::GetGammaRamp (
    UINT iSwapChain,
    D3DGAMMARAMP* pRamp)
{
    return mRealDevice->GetGammaRamp (iSwapChain, pRamp);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::CreateTexture (
    UINT Width,
    UINT Height,
    UINT Levels,
    DWORD Usage,
    D3DFORMAT Format,
    D3DPOOL Pool,
    IDirect3DTexture9** ppTexture,
    HANDLE* pSharedHandle)
{
    return mRealDevice->CreateTexture (
        Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::CreateVolumeTexture (
    UINT Width,
    UINT Height,
    UINT Depth,
    UINT Levels,
    DWORD Usage,
    D3DFORMAT Format,
    D3DPOOL Pool,
    IDirect3DVolumeTexture9** ppVolumeTexture,
    HANDLE* pSharedHandle)
{
    return mRealDevice->CreateVolumeTexture (
        Width, Height, Depth, Levels, Usage, Format,
        Pool, ppVolumeTexture, pSharedHandle);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::CreateCubeTexture (
    UINT EdgeLength,
    UINT Levels,
    DWORD Usage,
    D3DFORMAT Format,
    D3DPOOL Pool,
    IDirect3DCubeTexture9** ppCubeTexture,
    HANDLE* pSharedHandle)
{
    return mRealDevice->CreateCubeTexture (
        EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pSharedHandle);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::CreateVertexBuffer (
    UINT Length,
    DWORD Usage,
    DWORD FVF,
    D3DPOOL Pool,
    IDirect3DVertexBuffer9** ppVertexBuffer,
    HANDLE* pSharedHandle)
{
    return mRealDevice->CreateVertexBuffer (
        Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::CreateIndexBuffer (
    UINT Length,
    DWORD Usage,
    D3DFORMAT Format,
    D3DPOOL Pool,
    IDirect3DIndexBuffer9** ppIndexBuffer,
    HANDLE* pSharedHandle)
{
    return mRealDevice->CreateIndexBuffer (
        Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::CreateRenderTarget (
    UINT Width,
    UINT Height,
    D3DFORMAT Format,
    D3DMULTISAMPLE_TYPE MultiSample,
    DWORD MultisampleQuality,
    BOOL Lockable,
    IDirect3DSurface9** ppSurface,
    HANDLE* pSharedHandle)
{
    return mRealDevice->CreateRenderTarget (
        Width, Height, Format, MultiSample, MultisampleQuality,
        Lockable, ppSurface, pSharedHandle);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::CreateDepthStencilSurface (
    UINT Width,
    UINT Height,
    D3DFORMAT Format,
    D3DMULTISAMPLE_TYPE MultiSample,
    DWORD MultisampleQuality,
    BOOL Discard,
    IDirect3DSurface9** ppSurface,
    HANDLE* pSharedHandle)
{
    return mRealDevice->CreateDepthStencilSurface (
        Width, Height, Format, MultiSample, MultisampleQuality,
        Discard, ppSurface, pSharedHandle);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::UpdateSurface (
    IDirect3DSurface9* pSourceSurface,
    CONST RECT* pSourceRect,
    IDirect3DSurface9* pDestinationSurface,
    CONST POINT* pDestPoint)
{
    return mRealDevice->UpdateSurface (
        pSourceSurface, pSourceRect, pDestinationSurface, pDestPoint);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::UpdateTexture (
    IDirect3DBaseTexture9* pSourceTexture,
    IDirect3DBaseTexture9* pDestinationTexture)
{
    return mRealDevice->UpdateTexture (pSourceTexture, pDestinationTexture);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetRenderTargetData (
    IDirect3DSurface9* pRenderTarget,
    IDirect3DSurface9* pDestSurface)
{
    return mRealDevice->GetRenderTargetData (pRenderTarget, pDestSurface);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetFrontBufferData (
    UINT iSwapChain,
    IDirect3DSurface9* pDestSurface)
{
    return mRealDevice->GetFrontBufferData (iSwapChain, pDestSurface);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::StretchRect (
    IDirect3DSurface9* pSourceSurface,
    CONST RECT* pSourceRect,
    IDirect3DSurface9* pDestSurface,
    CONST RECT* pDestRect,
    D3DTEXTUREFILTERTYPE Filter)
{
    return mRealDevice->StretchRect (
        pSourceSurface, pSourceRect, pDestSurface, pDestRect, Filter);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::ColorFill (
    IDirect3DSurface9* pSurface,
    CONST RECT* pRect,
    D3DCOLOR color)
{
    return mRealDevice->ColorFill (pSurface, pRect, color);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::CreateOffscreenPlainSurface (
    UINT Width,
    UINT Height,
    D3DFORMAT Format,
    D3DPOOL Pool,
    IDirect3DSurface9** ppSurface,
    HANDLE* pSharedHandle)
{
    return mRealDevice->CreateOffscreenPlainSurface (
        Width, Height, Format, Pool, ppSurface, pSharedHandle);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetRenderTarget (
    DWORD RenderTargetIndex,
    IDirect3DSurface9* pRenderTarget)
{
    return mRealDevice->SetRenderTarget (RenderTargetIndex, pRenderTarget);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetRenderTarget (
    DWORD RenderTargetIndex,
    IDirect3DSurface9** ppRenderTarget)
{
    return mRealDevice->GetRenderTarget (RenderTargetIndex, ppRenderTarget);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetDepthStencilSurface (
    IDirect3DSurface9* pNewZStencil)
{
    return mRealDevice->SetDepthStencilSurface (pNewZStencil);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetDepthStencilSurface (
    IDirect3DSurface9** ppZStencilSurface)
{
    return mRealDevice->GetDepthStencilSurface (ppZStencilSurface);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::BeginScene ()
{
    return mRealDevice->BeginScene ();
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::EndScene ()
{
    drawSubtitle ();

    return mRealDevice->EndScene ();
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::Clear (
    DWORD Count,
    CONST D3DRECT* pRects,
    DWORD Flags,
    D3DCOLOR Color,
    float Z,
    DWORD Stencil)
{
    return mRealDevice->Clear (Count, pRects, Flags, Color, Z, Stencil);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetTransform (
    D3DTRANSFORMSTATETYPE State,
    CONST D3DMATRIX* pMatrix)
{
    return mRealDevice->SetTransform (State, pMatrix);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetTransform (
    D3DTRANSFORMSTATETYPE State,
    D3DMATRIX* pMatrix)
{
    return mRealDevice->GetTransform (State, pMatrix);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::MultiplyTransform (
    D3DTRANSFORMSTATETYPE State,
    CONST D3DMATRIX* pMatrix)
{
    return mRealDevice->MultiplyTransform (State, pMatrix);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetViewport (
    CONST D3DVIEWPORT9* pViewport)
{
    return mRealDevice->SetViewport (pViewport);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetViewport (
    D3DVIEWPORT9* pViewport)
{
    return mRealDevice->GetViewport (pViewport);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetMaterial (
    CONST D3DMATERIAL9* pMaterial)
{
    return mRealDevice->SetMaterial (pMaterial);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetMaterial (
    D3DMATERIAL9* pMaterial)
{
    return mRealDevice->GetMaterial (pMaterial);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetLight (
    DWORD Index,
    CONST D3DLIGHT9* pLight)
{
    return mRealDevice->SetLight (Index, pLight);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetLight (
    DWORD Index,
    D3DLIGHT9* pLight)
{
    return mRealDevice->GetLight (Index, pLight);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::LightEnable (
    DWORD Index,
    BOOL Enable)
{
    return mRealDevice->LightEnable (Index, Enable);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetLightEnable (
    DWORD Index,
    BOOL* pEnable)
{
    return mRealDevice->GetLightEnable (Index, pEnable);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetClipPlane (
    DWORD Index,
    CONST float* pPlane)
{
    return mRealDevice->SetClipPlane (Index, pPlane);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetClipPlane (
    DWORD Index,
    float* pPlane)
{
    return mRealDevice->GetClipPlane (Index, pPlane);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetRenderState (
    D3DRENDERSTATETYPE State,
    DWORD Value)
{
    return mRealDevice->SetRenderState (State, Value);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetRenderState (
    D3DRENDERSTATETYPE State,
    DWORD* pValue)
{
    return mRealDevice->GetRenderState (State, pValue);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::CreateStateBlock (
    D3DSTATEBLOCKTYPE Type,
    IDirect3DStateBlock9** ppSB)
{
    return mRealDevice->CreateStateBlock (Type, ppSB);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::BeginStateBlock ( )
{
    return mRealDevice->BeginStateBlock ();
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::EndStateBlock (
    IDirect3DStateBlock9** ppSB)
{
    return mRealDevice->EndStateBlock (ppSB);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetClipStatus (
    CONST D3DCLIPSTATUS9* pClipStatus)
{
    return mRealDevice->SetClipStatus (pClipStatus);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetClipStatus (
    D3DCLIPSTATUS9* pClipStatus)
{
    return mRealDevice->GetClipStatus (pClipStatus);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetTexture (
    DWORD Stage,
    IDirect3DBaseTexture9** ppTexture)
{
    return mRealDevice->GetTexture (Stage, ppTexture);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetTexture (
    DWORD Stage,
    IDirect3DBaseTexture9* pTexture)
{
    return mRealDevice->SetTexture (Stage, pTexture);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetTextureStageState (
    DWORD Stage,
    D3DTEXTURESTAGESTATETYPE Type,
    DWORD* pValue)
{
    return mRealDevice->GetTextureStageState (Stage, Type, pValue);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetTextureStageState (
    DWORD Stage,
    D3DTEXTURESTAGESTATETYPE Type,
    DWORD Value)
{
    return mRealDevice->SetTextureStageState (Stage, Type, Value);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetSamplerState (
    DWORD Sampler,
    D3DSAMPLERSTATETYPE Type,
    DWORD* pValue)
{
    return mRealDevice->GetSamplerState (Sampler, Type, pValue);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetSamplerState (
    DWORD Sampler,
    D3DSAMPLERSTATETYPE Type,
    DWORD Value)
{
    return mRealDevice->SetSamplerState (Sampler, Type, Value);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::ValidateDevice (
    DWORD* pNumPasses)
{
    return mRealDevice->ValidateDevice (pNumPasses);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetPaletteEntries (
    UINT PaletteNumber,
    CONST PALETTEENTRY* pEntries)
{
    return mRealDevice->SetPaletteEntries (PaletteNumber, pEntries);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetPaletteEntries (
    UINT PaletteNumber,
    PALETTEENTRY* pEntries)
{
    return mRealDevice->GetPaletteEntries (PaletteNumber, pEntries);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetCurrentTexturePalette (
    UINT PaletteNumber)
{
    return mRealDevice->SetCurrentTexturePalette (PaletteNumber);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetCurrentTexturePalette (
    UINT *PaletteNumber)
{
    return mRealDevice->GetCurrentTexturePalette (PaletteNumber);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetScissorRect (
    CONST RECT* pRect)
{
    return mRealDevice->SetScissorRect (pRect);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetScissorRect (
    RECT* pRect)
{
    return mRealDevice->GetScissorRect (pRect);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetSoftwareVertexProcessing (
    BOOL bSoftware)
{
    return mRealDevice->SetSoftwareVertexProcessing (bSoftware);
}

BOOL STDMETHODCALLTYPE FakeDirect3dDevice9::GetSoftwareVertexProcessing ()
{
    return mRealDevice->GetSoftwareVertexProcessing ();
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetNPatchMode (
    float nSegments)
{
    return mRealDevice->SetNPatchMode (nSegments);
}

float STDMETHODCALLTYPE FakeDirect3dDevice9::GetNPatchMode ()
{
    return mRealDevice->GetNPatchMode ();
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::DrawPrimitive (
    D3DPRIMITIVETYPE PrimitiveType,
    UINT StartVertex,
    UINT PrimitiveCount)
{
    return mRealDevice->DrawPrimitive (PrimitiveType, StartVertex, PrimitiveCount);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::DrawIndexedPrimitive (
    D3DPRIMITIVETYPE PrimitiveType,
    INT BaseVertexIndex,
    UINT MinVertexIndex,
    UINT NumVertices,
    UINT startIndex,
    UINT primCount)
{
    return mRealDevice->DrawIndexedPrimitive (
        PrimitiveType, BaseVertexIndex, MinVertexIndex,
        NumVertices, startIndex, primCount);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::DrawPrimitiveUP (
    D3DPRIMITIVETYPE PrimitiveType,
    UINT PrimitiveCount,
    CONST void* pVertexStreamZeroData,
    UINT VertexStreamZeroStride)
{
    return mRealDevice->DrawPrimitiveUP (
        PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::DrawIndexedPrimitiveUP (
    D3DPRIMITIVETYPE PrimitiveType,
    UINT MinVertexIndex,
    UINT NumVertices,
    UINT PrimitiveCount,
    CONST void* pIndexData,
    D3DFORMAT IndexDataFormat,
    CONST void* pVertexStreamZeroData,
    UINT VertexStreamZeroStride)
{
    return mRealDevice->DrawIndexedPrimitiveUP (
        PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData,
        IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::ProcessVertices (
    UINT SrcStartIndex,
    UINT DestIndex,
    UINT VertexCount,
    IDirect3DVertexBuffer9* pDestBuffer,
    IDirect3DVertexDeclaration9* pVertexDecl,
    DWORD Flags)
{
    return mRealDevice->ProcessVertices (
        SrcStartIndex, DestIndex, VertexCount, pDestBuffer, pVertexDecl, Flags);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::CreateVertexDeclaration (
    CONST D3DVERTEXELEMENT9* pVertexElements,
    IDirect3DVertexDeclaration9** ppDecl)
{
    return mRealDevice->CreateVertexDeclaration (pVertexElements, ppDecl);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetVertexDeclaration (
    IDirect3DVertexDeclaration9* pDecl)
{
    return mRealDevice->SetVertexDeclaration (pDecl);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetVertexDeclaration (
    IDirect3DVertexDeclaration9** ppDecl)
{
    return mRealDevice->GetVertexDeclaration (ppDecl);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetFVF (
    DWORD FVF)
{
    return mRealDevice->SetFVF (FVF);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetFVF (
    DWORD* pFVF)
{
    return mRealDevice->GetFVF (pFVF);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::CreateVertexShader (
    CONST DWORD* pFunction,
    IDirect3DVertexShader9** ppShader)
{
    return mRealDevice->CreateVertexShader (pFunction, ppShader);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetVertexShader (
    IDirect3DVertexShader9* pShader)
{
    return mRealDevice->SetVertexShader (pShader);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetVertexShader (
    IDirect3DVertexShader9** ppShader)
{
    return mRealDevice->GetVertexShader (ppShader);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetVertexShaderConstantF (
    UINT StartRegister,
    CONST float* pConstantData,
    UINT Vector4fCount)
{
    return mRealDevice->SetVertexShaderConstantF (
        StartRegister, pConstantData, Vector4fCount);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetVertexShaderConstantF (
    UINT StartRegister,
    float* pConstantData,
    UINT Vector4fCount)
{
    return mRealDevice->GetVertexShaderConstantF (
        StartRegister, pConstantData, Vector4fCount);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetVertexShaderConstantI (
    UINT StartRegister,
    CONST int* pConstantData,
    UINT Vector4iCount)
{
    return mRealDevice->SetVertexShaderConstantI (
        StartRegister, pConstantData, Vector4iCount);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetVertexShaderConstantI (
    UINT StartRegister,
    int* pConstantData,
    UINT Vector4iCount)
{
    return mRealDevice->GetVertexShaderConstantI (
        StartRegister, pConstantData, Vector4iCount);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetVertexShaderConstantB (
    UINT StartRegister,
    CONST BOOL* pConstantData,
    UINT  BoolCount)
{
    return mRealDevice->SetVertexShaderConstantB (
        StartRegister, pConstantData, BoolCount);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetVertexShaderConstantB (
    UINT StartRegister,
    BOOL* pConstantData,
    UINT BoolCount)
{
    return mRealDevice->GetVertexShaderConstantB (
        StartRegister, pConstantData, BoolCount);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetStreamSource (
    UINT StreamNumber,
    IDirect3DVertexBuffer9* pStreamData,
    UINT OffsetInBytes,
    UINT Stride)
{
    return mRealDevice->SetStreamSource (
        StreamNumber, pStreamData, OffsetInBytes, Stride);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetStreamSource (
    UINT StreamNumber,
    IDirect3DVertexBuffer9** ppStreamData,
    UINT* pOffsetInBytes,
    UINT* pStride)
{
    return mRealDevice->GetStreamSource (
        StreamNumber, ppStreamData, pOffsetInBytes, pStride);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetStreamSourceFreq (
    UINT StreamNumber,
    UINT Setting)
{
    return mRealDevice->SetStreamSourceFreq (StreamNumber, Setting);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetStreamSourceFreq (
    UINT StreamNumber,
    UINT* pSetting)
{
    return mRealDevice->GetStreamSourceFreq (StreamNumber, pSetting);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetIndices (
    IDirect3DIndexBuffer9* pIndexData)
{
    return mRealDevice->SetIndices (pIndexData);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetIndices (
    IDirect3DIndexBuffer9** ppIndexData)
{
    return mRealDevice->GetIndices (ppIndexData);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::CreatePixelShader (
    CONST DWORD* pFunction,
    IDirect3DPixelShader9** ppShader)
{
    return mRealDevice->CreatePixelShader (pFunction, ppShader);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetPixelShader (
    IDirect3DPixelShader9* pShader)
{
    return mRealDevice->SetPixelShader (pShader);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetPixelShader (
    IDirect3DPixelShader9** ppShader)
{
    return mRealDevice->GetPixelShader (ppShader);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetPixelShaderConstantF (
    UINT StartRegister,
    CONST float* pConstantData,
    UINT Vector4fCount)
{
    return mRealDevice->SetPixelShaderConstantF (
        StartRegister, pConstantData, Vector4fCount);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetPixelShaderConstantF (
    UINT StartRegister,
    float* pConstantData,
    UINT Vector4fCount)
{
    return mRealDevice->GetPixelShaderConstantF (
        StartRegister, pConstantData, Vector4fCount);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetPixelShaderConstantI (
    UINT StartRegister,
    CONST int* pConstantData,
    UINT Vector4iCount)
{
    return mRealDevice->SetPixelShaderConstantI (
        StartRegister, pConstantData, Vector4iCount);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetPixelShaderConstantI (
    UINT StartRegister,
    int* pConstantData,
    UINT Vector4iCount)
{
    return mRealDevice->GetPixelShaderConstantI (
        StartRegister, pConstantData, Vector4iCount);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetPixelShaderConstantB (
    UINT StartRegister,
    CONST BOOL* pConstantData,
    UINT  BoolCount)
{
    return mRealDevice->SetPixelShaderConstantB (
        StartRegister, pConstantData, BoolCount);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetPixelShaderConstantB (
    UINT StartRegister,
    BOOL* pConstantData,
    UINT BoolCount)
{
    return mRealDevice->GetPixelShaderConstantB (
        StartRegister, pConstantData, BoolCount);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::DrawRectPatch (
    UINT Handle,
    CONST float* pNumSegs,
    CONST D3DRECTPATCH_INFO* pRectPatchInfo)
{
    return mRealDevice->DrawRectPatch (Handle, pNumSegs, pRectPatchInfo);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::DrawTriPatch (
    UINT Handle,
    CONST float* pNumSegs,
    CONST D3DTRIPATCH_INFO* pTriPatchInfo)
{
    return mRealDevice->DrawTriPatch (Handle, pNumSegs, pTriPatchInfo);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::DeletePatch (
    UINT Handle)
{
    return mRealDevice->DeletePatch (Handle);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::CreateQuery (
    D3DQUERYTYPE Type,
    IDirect3DQuery9** ppQuery)
{
    return mRealDevice->CreateQuery (Type, ppQuery);
}

void FakeDirect3dDevice9::calculateProps ()
{
    const bbi::DllContext& context = LgVidContext;

    if (!context.showSubs)
        return;


    mFontHeight = 0;
    mTextHeight = 0;
    mSpaceAfter = 0;


    HRESULT d3dResult;

    d3dResult = mRealDevice->GetViewport (&mViewPort);

    if (d3dResult != D3D_OK)
        return;


    float fontHeightF = context.fontSize;

    if (context.fontSizeInPercents)
        fontHeightF *= mViewPort.Height * 0.01F;

    mFontHeight = static_cast<int> (fontHeightF);


    mFontWeight = static_cast<int> (context.fontWeight);


    float spaceAfterF = context.spaceAfter;

    if (context.spaceAfterInPercents)
        spaceAfterF *= mViewPort.Height * 0.01F;

    if (spaceAfterF < 0.0F)
        spaceAfterF = 0.0F;

    mSpaceAfter = static_cast<int> (spaceAfterF);


    float shadowOffsetXF = context.shadowOffsetX;

    if (context.shadowOffsetXInPercents)
        shadowOffsetXF *= mViewPort.Height * 0.01F;

    mShadowOffsetX = static_cast<int> (shadowOffsetXF);


    float shadowOffsetYF = context.shadowOffsetY;

    if (context.shadowOffsetYInPercents)
        shadowOffsetYF *= mViewPort.Height * 0.01F;

    mShadowOffsetY = static_cast<int> (shadowOffsetYF);
}

void FakeDirect3dDevice9::measureText ()
{
    const bbi::DllContext& context = LgVidContext;


    RECT rect;
    HRESULT d3dResult;

    mTextHeight = 0;

    for (int i = 0; i < context.lineCount; ++i) {
        const wchar_t* line = context.lines[i];
        int lineLength = context.linesLengths[i];
        SIZE& lineSize = mLinesSizes[i];

        if (lineLength > 0) {
            rect.left = 0;
            rect.top = 0;
            rect.right = 0;
            rect.bottom = 0;

            d3dResult = mFont->PreloadText (line, lineLength);

            INT textResult = mFont->DrawTextW (
                mSprite, // sprite
                line, // text
                lineLength, // count,
                &rect, // rectangle
                DT_NOCLIP | DT_LEFT | DT_TOP | DT_SINGLELINE | DT_CALCRECT, // format
                0 // color
            );

            if (textResult == 0)
                textResult = mFontHeight - 1;

            lineSize.cy = textResult + 1;
            lineSize.cx = rect.right + 1;

            mTextHeight += lineSize.cy;
        } else {
            mTextHeight += mFontHeight;
            lineSize.cx = 0;
            lineSize.cy = 0;
        }
    }
}

void FakeDirect3dDevice9::drawSubtitle ()
{
    if (mIsDeviceLost)
        return;


    const bbi::DllContext& context = LgVidContext;

    if (!context.showSubs) {
        bbi::ComHelper::releaseAndNull (mFont);
        bbi::ComHelper::releaseAndNull (mSprite);
        return;
    }


    int lineCount = context.lineCount;

    if (lineCount <= 0)
        return;


    HRESULT d3dResult = D3D_OK;

    if (mSprite == 0) {
        d3dResult = mD3dX9Funcs->createSprite (
            mRealDevice,
            &mSprite);
    }

    if (mFont == 0) {
        calculateProps ();

        if (mFontHeight > 0) {
            d3dResult = mD3dX9Funcs->createFont (
                mRealDevice,
                mFontHeight, // height
                0, // width
                mFontWeight, // weight
                0, // mipmap levels
                FALSE, // italic?
                DEFAULT_CHARSET, // charset
                OUT_TT_ONLY_PRECIS, // precision
                DEFAULT_QUALITY, // quality
                FF_DONTCARE | VARIABLE_PITCH, // pitch and family
                context.fontFamily, // typeface name
                &mFont // result
            );
        }
    }

    if (mFont == 0)
        return;


    if (mOldSubIndex != context.subIndex) {
        measureText ();
        mOldSubIndex = context.subIndex;
    }

    if (mSprite != 0)
        d3dResult = mSprite->Begin (D3DXSPRITE_ALPHABLEND);

    INT textResult = 0;

    RECT rect;


    int baseX = mViewPort.X;
    int y = mViewPort.Y + mViewPort.Height - mTextHeight - mSpaceAfter;


    for (int i = 0; i < lineCount; ++i) {
        const wchar_t* line = context.lines[i];
        int lineLength = context.linesLengths[i];
        const SIZE& lineSize = mLinesSizes[i];

        if ((lineLength > 0) && ((lineSize.cx > 0) && (lineSize.cy > 0))) {
            rect.left = baseX + (static_cast<LONG> (mViewPort.Width) - lineSize.cx) / 2;
            rect.top = y;
            rect.right = rect.left + lineSize.cx;
            rect.bottom = rect.top + lineSize.cy;


            if ((mShadowOffsetX != 0) && (mShadowOffsetY != 0)) {
                rect.left += mShadowOffsetX;
                rect.top += mShadowOffsetY;

                textResult = mFont->DrawTextW (
                    mSprite, // sprite
                    line, // text
                    lineLength, // count,
                    &rect, // rectangle
                    DT_NOCLIP | DT_LEFT | DT_TOP | DT_SINGLELINE, // format
                    context.shadowColor // color
                );

                rect.left -= mShadowOffsetX;
                rect.top -= mShadowOffsetY;
            }

            textResult = mFont->DrawTextW (
                mSprite, // sprite
                line, // text
                lineLength, // count,
                &rect, // rectangle
                DT_NOCLIP | DT_LEFT | DT_TOP | DT_SINGLELINE, // format
                context.fontColor // color
            );

            y += lineSize.cy;
        } else
            y += mFontHeight;
    }

    if (mSprite != 0)
        d3dResult = mSprite->End ();
}
