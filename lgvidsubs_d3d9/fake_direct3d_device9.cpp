#include "bbi_com_helper.h"
#include "bbi_dll_context.h"
#include "bbi_math.h"
#include "fake_direct3d_device9.h"


extern bbi::DllContext LgVidContext;


FakeDirect3dDevice9::FakeDirect3dDevice9 (
    IDirect3DDevice9* real_device,
    D3dX9Funcs* d3dx9_funcs) :
        real_device_ (real_device),
        is_device_lost_ (false),
        d3dx9_funcs_ (d3dx9_funcs),
        font_ (0),
        sprite_ (0),
        old_sub_index_ (-1),
        font_height_ (0),
        font_weight_ (0),
        text_height_ (0),
        space_after_ (0),
        shadow_offset_x_ (0),
        shadow_offset_y_ (0)
{
}

FakeDirect3dDevice9::~FakeDirect3dDevice9 ()
{
    bbi::ComHelper::release_and_null (font_);
    bbi::ComHelper::release_and_null (sprite_);
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

    return real_device_->QueryInterface (riid, ppvObj);
}

ULONG STDMETHODCALLTYPE FakeDirect3dDevice9::AddRef ()
{
    return real_device_->AddRef ();
}

ULONG STDMETHODCALLTYPE FakeDirect3dDevice9::Release ()
{
    ULONG result = real_device_->Release ();

    if (result == 0)
        delete this;

    return result;
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::TestCooperativeLevel ()
{
    HRESULT result = real_device_->TestCooperativeLevel ();

    is_device_lost_ = (result != D3D_OK);

    if (is_device_lost_) {
        HRESULT d3dResult = D3D_OK;

        if (sprite_ != 0)
            d3dResult = sprite_->OnLostDevice ();

        if (font_ != 0)
            d3dResult = font_->OnLostDevice ();
    }

    return result;
}

UINT STDMETHODCALLTYPE FakeDirect3dDevice9::GetAvailableTextureMem ()
{
    return real_device_->GetAvailableTextureMem ();
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::EvictManagedResources ()
{
    return real_device_->EvictManagedResources ();
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetDirect3D (
    IDirect3D9** ppD3D9)
{
    return real_device_->GetDirect3D (ppD3D9);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetDeviceCaps (
    D3DCAPS9* pCaps)
{
    return real_device_->GetDeviceCaps (pCaps);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetDisplayMode (
    UINT iSwapChain,
    D3DDISPLAYMODE* pMode)
{
    return real_device_->GetDisplayMode (iSwapChain, pMode);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetCreationParameters (
    D3DDEVICE_CREATION_PARAMETERS *pParameters)
{
    return real_device_->GetCreationParameters (pParameters);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetCursorProperties (
    UINT XHotSpot,
    UINT YHotSpot,
    IDirect3DSurface9* pCursorBitmap)
{
    return real_device_->SetCursorProperties (XHotSpot, YHotSpot, pCursorBitmap);
}

void STDMETHODCALLTYPE FakeDirect3dDevice9::SetCursorPosition (
    int X,
    int Y,
    DWORD Flags)
{
    return real_device_->SetCursorPosition (X, Y, Flags);
}

BOOL STDMETHODCALLTYPE FakeDirect3dDevice9::ShowCursor (
    BOOL bShow)
{
    return real_device_->ShowCursor (bShow);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::CreateAdditionalSwapChain (
    D3DPRESENT_PARAMETERS* pPresentationParameters,
    IDirect3DSwapChain9** pSwapChain)
{
    return real_device_->CreateAdditionalSwapChain (pPresentationParameters, pSwapChain);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetSwapChain (
    UINT iSwapChain,
    IDirect3DSwapChain9** pSwapChain)
{
    return real_device_->GetSwapChain (iSwapChain, pSwapChain);
}

UINT STDMETHODCALLTYPE FakeDirect3dDevice9::GetNumberOfSwapChains ( )
{
    return real_device_->GetNumberOfSwapChains ();
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::Reset (
    D3DPRESENT_PARAMETERS* pPresentationParameters)
{
    HRESULT d3dResult = D3D_OK;

    if (LgVidContext.show_subs) {
        if (sprite_ != 0)
            d3dResult = sprite_->OnLostDevice ();

        if (font_ != 0)
            d3dResult = font_->OnLostDevice ();
    }

    HRESULT result = real_device_->Reset (pPresentationParameters);

    if (LgVidContext.show_subs)
        measure_text ();

    return result;
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::Present (
    CONST RECT* pSourceRect,
    CONST RECT* pDestRect,
    HWND hDestWindowOverride,
    CONST RGNDATA* pDirtyRegion)
{
    return real_device_->Present (
        pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetBackBuffer (
    UINT iSwapChain,
    UINT iBackBuffer,
    D3DBACKBUFFER_TYPE Type,
    IDirect3DSurface9** ppBackBuffer)
{
    return real_device_->GetBackBuffer (iSwapChain, iBackBuffer, Type, ppBackBuffer);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetRasterStatus (
    UINT iSwapChain,
    D3DRASTER_STATUS* pRasterStatus)
{
    return real_device_->GetRasterStatus (iSwapChain, pRasterStatus);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetDialogBoxMode (
    BOOL bEnableDialogs)
{
    return real_device_->SetDialogBoxMode (bEnableDialogs);
}

void STDMETHODCALLTYPE FakeDirect3dDevice9::SetGammaRamp (
    UINT iSwapChain,
    DWORD Flags,
    CONST D3DGAMMARAMP* pRamp)
{
    return real_device_->SetGammaRamp (iSwapChain, Flags, pRamp);
}

void STDMETHODCALLTYPE FakeDirect3dDevice9::GetGammaRamp (
    UINT iSwapChain,
    D3DGAMMARAMP* pRamp)
{
    return real_device_->GetGammaRamp (iSwapChain, pRamp);
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
    return real_device_->CreateTexture (
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
    return real_device_->CreateVolumeTexture (
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
    return real_device_->CreateCubeTexture (
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
    return real_device_->CreateVertexBuffer (
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
    return real_device_->CreateIndexBuffer (
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
    return real_device_->CreateRenderTarget (
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
    return real_device_->CreateDepthStencilSurface (
        Width, Height, Format, MultiSample, MultisampleQuality,
        Discard, ppSurface, pSharedHandle);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::UpdateSurface (
    IDirect3DSurface9* pSourceSurface,
    CONST RECT* pSourceRect,
    IDirect3DSurface9* pDestinationSurface,
    CONST POINT* pDestPoint)
{
    return real_device_->UpdateSurface (
        pSourceSurface, pSourceRect, pDestinationSurface, pDestPoint);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::UpdateTexture (
    IDirect3DBaseTexture9* pSourceTexture,
    IDirect3DBaseTexture9* pDestinationTexture)
{
    return real_device_->UpdateTexture (pSourceTexture, pDestinationTexture);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetRenderTargetData (
    IDirect3DSurface9* pRenderTarget,
    IDirect3DSurface9* pDestSurface)
{
    return real_device_->GetRenderTargetData (pRenderTarget, pDestSurface);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetFrontBufferData (
    UINT iSwapChain,
    IDirect3DSurface9* pDestSurface)
{
    return real_device_->GetFrontBufferData (iSwapChain, pDestSurface);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::StretchRect (
    IDirect3DSurface9* pSourceSurface,
    CONST RECT* pSourceRect,
    IDirect3DSurface9* pDestSurface,
    CONST RECT* pDestRect,
    D3DTEXTUREFILTERTYPE Filter)
{
    return real_device_->StretchRect (
        pSourceSurface, pSourceRect, pDestSurface, pDestRect, Filter);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::ColorFill (
    IDirect3DSurface9* pSurface,
    CONST RECT* pRect,
    D3DCOLOR color)
{
    return real_device_->ColorFill (pSurface, pRect, color);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::CreateOffscreenPlainSurface (
    UINT Width,
    UINT Height,
    D3DFORMAT Format,
    D3DPOOL Pool,
    IDirect3DSurface9** ppSurface,
    HANDLE* pSharedHandle)
{
    return real_device_->CreateOffscreenPlainSurface (
        Width, Height, Format, Pool, ppSurface, pSharedHandle);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetRenderTarget (
    DWORD RenderTargetIndex,
    IDirect3DSurface9* pRenderTarget)
{
    return real_device_->SetRenderTarget (RenderTargetIndex, pRenderTarget);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetRenderTarget (
    DWORD RenderTargetIndex,
    IDirect3DSurface9** ppRenderTarget)
{
    return real_device_->GetRenderTarget (RenderTargetIndex, ppRenderTarget);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetDepthStencilSurface (
    IDirect3DSurface9* pNewZStencil)
{
    return real_device_->SetDepthStencilSurface (pNewZStencil);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetDepthStencilSurface (
    IDirect3DSurface9** ppZStencilSurface)
{
    return real_device_->GetDepthStencilSurface (ppZStencilSurface);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::BeginScene ()
{
    return real_device_->BeginScene ();
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::EndScene ()
{
    draw_subtitle ();

    return real_device_->EndScene ();
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::Clear (
    DWORD Count,
    CONST D3DRECT* pRects,
    DWORD Flags,
    D3DCOLOR Color,
    float Z,
    DWORD Stencil)
{
    return real_device_->Clear (Count, pRects, Flags, Color, Z, Stencil);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetTransform (
    D3DTRANSFORMSTATETYPE State,
    CONST D3DMATRIX* pMatrix)
{
    return real_device_->SetTransform (State, pMatrix);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetTransform (
    D3DTRANSFORMSTATETYPE State,
    D3DMATRIX* pMatrix)
{
    return real_device_->GetTransform (State, pMatrix);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::MultiplyTransform (
    D3DTRANSFORMSTATETYPE State,
    CONST D3DMATRIX* pMatrix)
{
    return real_device_->MultiplyTransform (State, pMatrix);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetViewport (
    CONST D3DVIEWPORT9* pViewport)
{
    return real_device_->SetViewport (pViewport);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetViewport (
    D3DVIEWPORT9* pViewport)
{
    return real_device_->GetViewport (pViewport);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetMaterial (
    CONST D3DMATERIAL9* pMaterial)
{
    return real_device_->SetMaterial (pMaterial);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetMaterial (
    D3DMATERIAL9* pMaterial)
{
    return real_device_->GetMaterial (pMaterial);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetLight (
    DWORD Index,
    CONST D3DLIGHT9* pLight)
{
    return real_device_->SetLight (Index, pLight);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetLight (
    DWORD Index,
    D3DLIGHT9* pLight)
{
    return real_device_->GetLight (Index, pLight);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::LightEnable (
    DWORD Index,
    BOOL Enable)
{
    return real_device_->LightEnable (Index, Enable);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetLightEnable (
    DWORD Index,
    BOOL* pEnable)
{
    return real_device_->GetLightEnable (Index, pEnable);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetClipPlane (
    DWORD Index,
    CONST float* pPlane)
{
    return real_device_->SetClipPlane (Index, pPlane);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetClipPlane (
    DWORD Index,
    float* pPlane)
{
    return real_device_->GetClipPlane (Index, pPlane);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetRenderState (
    D3DRENDERSTATETYPE State,
    DWORD Value)
{
    return real_device_->SetRenderState (State, Value);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetRenderState (
    D3DRENDERSTATETYPE State,
    DWORD* pValue)
{
    return real_device_->GetRenderState (State, pValue);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::CreateStateBlock (
    D3DSTATEBLOCKTYPE Type,
    IDirect3DStateBlock9** ppSB)
{
    return real_device_->CreateStateBlock (Type, ppSB);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::BeginStateBlock ( )
{
    return real_device_->BeginStateBlock ();
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::EndStateBlock (
    IDirect3DStateBlock9** ppSB)
{
    return real_device_->EndStateBlock (ppSB);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetClipStatus (
    CONST D3DCLIPSTATUS9* pClipStatus)
{
    return real_device_->SetClipStatus (pClipStatus);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetClipStatus (
    D3DCLIPSTATUS9* pClipStatus)
{
    return real_device_->GetClipStatus (pClipStatus);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetTexture (
    DWORD Stage,
    IDirect3DBaseTexture9** ppTexture)
{
    return real_device_->GetTexture (Stage, ppTexture);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetTexture (
    DWORD Stage,
    IDirect3DBaseTexture9* pTexture)
{
    return real_device_->SetTexture (Stage, pTexture);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetTextureStageState (
    DWORD Stage,
    D3DTEXTURESTAGESTATETYPE Type,
    DWORD* pValue)
{
    return real_device_->GetTextureStageState (Stage, Type, pValue);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetTextureStageState (
    DWORD Stage,
    D3DTEXTURESTAGESTATETYPE Type,
    DWORD Value)
{
    return real_device_->SetTextureStageState (Stage, Type, Value);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetSamplerState (
    DWORD Sampler,
    D3DSAMPLERSTATETYPE Type,
    DWORD* pValue)
{
    return real_device_->GetSamplerState (Sampler, Type, pValue);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetSamplerState (
    DWORD Sampler,
    D3DSAMPLERSTATETYPE Type,
    DWORD Value)
{
    return real_device_->SetSamplerState (Sampler, Type, Value);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::ValidateDevice (
    DWORD* pNumPasses)
{
    return real_device_->ValidateDevice (pNumPasses);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetPaletteEntries (
    UINT PaletteNumber,
    CONST PALETTEENTRY* pEntries)
{
    return real_device_->SetPaletteEntries (PaletteNumber, pEntries);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetPaletteEntries (
    UINT PaletteNumber,
    PALETTEENTRY* pEntries)
{
    return real_device_->GetPaletteEntries (PaletteNumber, pEntries);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetCurrentTexturePalette (
    UINT PaletteNumber)
{
    return real_device_->SetCurrentTexturePalette (PaletteNumber);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetCurrentTexturePalette (
    UINT *PaletteNumber)
{
    return real_device_->GetCurrentTexturePalette (PaletteNumber);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetScissorRect (
    CONST RECT* pRect)
{
    return real_device_->SetScissorRect (pRect);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetScissorRect (
    RECT* pRect)
{
    return real_device_->GetScissorRect (pRect);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetSoftwareVertexProcessing (
    BOOL bSoftware)
{
    return real_device_->SetSoftwareVertexProcessing (bSoftware);
}

BOOL STDMETHODCALLTYPE FakeDirect3dDevice9::GetSoftwareVertexProcessing ()
{
    return real_device_->GetSoftwareVertexProcessing ();
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetNPatchMode (
    float nSegments)
{
    return real_device_->SetNPatchMode (nSegments);
}

float STDMETHODCALLTYPE FakeDirect3dDevice9::GetNPatchMode ()
{
    return real_device_->GetNPatchMode ();
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::DrawPrimitive (
    D3DPRIMITIVETYPE PrimitiveType,
    UINT StartVertex,
    UINT PrimitiveCount)
{
    return real_device_->DrawPrimitive (PrimitiveType, StartVertex, PrimitiveCount);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::DrawIndexedPrimitive (
    D3DPRIMITIVETYPE PrimitiveType,
    INT BaseVertexIndex,
    UINT MinVertexIndex,
    UINT NumVertices,
    UINT startIndex,
    UINT primCount)
{
    return real_device_->DrawIndexedPrimitive (
        PrimitiveType, BaseVertexIndex, MinVertexIndex,
        NumVertices, startIndex, primCount);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::DrawPrimitiveUP (
    D3DPRIMITIVETYPE PrimitiveType,
    UINT PrimitiveCount,
    CONST void* pVertexStreamZeroData,
    UINT VertexStreamZeroStride)
{
    return real_device_->DrawPrimitiveUP (
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
    return real_device_->DrawIndexedPrimitiveUP (
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
    return real_device_->ProcessVertices (
        SrcStartIndex, DestIndex, VertexCount, pDestBuffer, pVertexDecl, Flags);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::CreateVertexDeclaration (
    CONST D3DVERTEXELEMENT9* pVertexElements,
    IDirect3DVertexDeclaration9** ppDecl)
{
    return real_device_->CreateVertexDeclaration (pVertexElements, ppDecl);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetVertexDeclaration (
    IDirect3DVertexDeclaration9* pDecl)
{
    return real_device_->SetVertexDeclaration (pDecl);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetVertexDeclaration (
    IDirect3DVertexDeclaration9** ppDecl)
{
    return real_device_->GetVertexDeclaration (ppDecl);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetFVF (
    DWORD FVF)
{
    return real_device_->SetFVF (FVF);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetFVF (
    DWORD* pFVF)
{
    return real_device_->GetFVF (pFVF);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::CreateVertexShader (
    CONST DWORD* pFunction,
    IDirect3DVertexShader9** ppShader)
{
    return real_device_->CreateVertexShader (pFunction, ppShader);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetVertexShader (
    IDirect3DVertexShader9* pShader)
{
    return real_device_->SetVertexShader (pShader);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetVertexShader (
    IDirect3DVertexShader9** ppShader)
{
    return real_device_->GetVertexShader (ppShader);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetVertexShaderConstantF (
    UINT StartRegister,
    CONST float* pConstantData,
    UINT Vector4fCount)
{
    return real_device_->SetVertexShaderConstantF (
        StartRegister, pConstantData, Vector4fCount);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetVertexShaderConstantF (
    UINT StartRegister,
    float* pConstantData,
    UINT Vector4fCount)
{
    return real_device_->GetVertexShaderConstantF (
        StartRegister, pConstantData, Vector4fCount);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetVertexShaderConstantI (
    UINT StartRegister,
    CONST int* pConstantData,
    UINT Vector4iCount)
{
    return real_device_->SetVertexShaderConstantI (
        StartRegister, pConstantData, Vector4iCount);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetVertexShaderConstantI (
    UINT StartRegister,
    int* pConstantData,
    UINT Vector4iCount)
{
    return real_device_->GetVertexShaderConstantI (
        StartRegister, pConstantData, Vector4iCount);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetVertexShaderConstantB (
    UINT StartRegister,
    CONST BOOL* pConstantData,
    UINT  BoolCount)
{
    return real_device_->SetVertexShaderConstantB (
        StartRegister, pConstantData, BoolCount);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetVertexShaderConstantB (
    UINT StartRegister,
    BOOL* pConstantData,
    UINT BoolCount)
{
    return real_device_->GetVertexShaderConstantB (
        StartRegister, pConstantData, BoolCount);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetStreamSource (
    UINT StreamNumber,
    IDirect3DVertexBuffer9* pStreamData,
    UINT OffsetInBytes,
    UINT Stride)
{
    return real_device_->SetStreamSource (
        StreamNumber, pStreamData, OffsetInBytes, Stride);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetStreamSource (
    UINT StreamNumber,
    IDirect3DVertexBuffer9** ppStreamData,
    UINT* pOffsetInBytes,
    UINT* pStride)
{
    return real_device_->GetStreamSource (
        StreamNumber, ppStreamData, pOffsetInBytes, pStride);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetStreamSourceFreq (
    UINT StreamNumber,
    UINT Setting)
{
    return real_device_->SetStreamSourceFreq (StreamNumber, Setting);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetStreamSourceFreq (
    UINT StreamNumber,
    UINT* pSetting)
{
    return real_device_->GetStreamSourceFreq (StreamNumber, pSetting);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetIndices (
    IDirect3DIndexBuffer9* pIndexData)
{
    return real_device_->SetIndices (pIndexData);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetIndices (
    IDirect3DIndexBuffer9** ppIndexData)
{
    return real_device_->GetIndices (ppIndexData);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::CreatePixelShader (
    CONST DWORD* pFunction,
    IDirect3DPixelShader9** ppShader)
{
    return real_device_->CreatePixelShader (pFunction, ppShader);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetPixelShader (
    IDirect3DPixelShader9* pShader)
{
    return real_device_->SetPixelShader (pShader);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetPixelShader (
    IDirect3DPixelShader9** ppShader)
{
    return real_device_->GetPixelShader (ppShader);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetPixelShaderConstantF (
    UINT StartRegister,
    CONST float* pConstantData,
    UINT Vector4fCount)
{
    return real_device_->SetPixelShaderConstantF (
        StartRegister, pConstantData, Vector4fCount);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetPixelShaderConstantF (
    UINT StartRegister,
    float* pConstantData,
    UINT Vector4fCount)
{
    return real_device_->GetPixelShaderConstantF (
        StartRegister, pConstantData, Vector4fCount);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetPixelShaderConstantI (
    UINT StartRegister,
    CONST int* pConstantData,
    UINT Vector4iCount)
{
    return real_device_->SetPixelShaderConstantI (
        StartRegister, pConstantData, Vector4iCount);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetPixelShaderConstantI (
    UINT StartRegister,
    int* pConstantData,
    UINT Vector4iCount)
{
    return real_device_->GetPixelShaderConstantI (
        StartRegister, pConstantData, Vector4iCount);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::SetPixelShaderConstantB (
    UINT StartRegister,
    CONST BOOL* pConstantData,
    UINT  BoolCount)
{
    return real_device_->SetPixelShaderConstantB (
        StartRegister, pConstantData, BoolCount);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::GetPixelShaderConstantB (
    UINT StartRegister,
    BOOL* pConstantData,
    UINT BoolCount)
{
    return real_device_->GetPixelShaderConstantB (
        StartRegister, pConstantData, BoolCount);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::DrawRectPatch (
    UINT Handle,
    CONST float* pNumSegs,
    CONST D3DRECTPATCH_INFO* pRectPatchInfo)
{
    return real_device_->DrawRectPatch (Handle, pNumSegs, pRectPatchInfo);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::DrawTriPatch (
    UINT Handle,
    CONST float* pNumSegs,
    CONST D3DTRIPATCH_INFO* pTriPatchInfo)
{
    return real_device_->DrawTriPatch (Handle, pNumSegs, pTriPatchInfo);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::DeletePatch (
    UINT Handle)
{
    return real_device_->DeletePatch (Handle);
}

HRESULT STDMETHODCALLTYPE FakeDirect3dDevice9::CreateQuery (
    D3DQUERYTYPE Type,
    IDirect3DQuery9** ppQuery)
{
    return real_device_->CreateQuery (Type, ppQuery);
}

void FakeDirect3dDevice9::calculate_props ()
{
    const bbi::DllContext& context = LgVidContext;

    if (!context.show_subs)
        return;


    font_height_ = 0;
    text_height_ = 0;
    space_after_ = 0;


    HRESULT d3dResult;

    d3dResult = real_device_->GetViewport (&view_port_);

    if (d3dResult != D3D_OK)
        return;


    float fontHeightF = context.font_size;

    if (context.font_size_in_percents)
        fontHeightF *= view_port_.Height * 0.01F;

    font_height_ = static_cast<int> (fontHeightF);


    font_weight_ = static_cast<int> (context.font_weight);


    float spaceAfterF = context.space_after;

    if (context.space_after_in_percents)
        spaceAfterF *= view_port_.Height * 0.01F;

    if (spaceAfterF < 0.0F)
        spaceAfterF = 0.0F;

    space_after_ = static_cast<int> (spaceAfterF);


    float shadow_offset_xf = context.shadow_offset_x;

    if (context.shadow_offset_x_in_percents)
        shadow_offset_xf *= view_port_.Height * 0.01F;

    shadow_offset_x_ = static_cast<int> (shadow_offset_xf);


    float shadowOffsetYF = context.shadow_offset_y;

    if (context.shadow_offset_y_in_percents)
        shadowOffsetYF *= view_port_.Height * 0.01F;

    shadow_offset_y_ = static_cast<int> (shadowOffsetYF);
}

void FakeDirect3dDevice9::measure_text ()
{
    const bbi::DllContext& context = LgVidContext;


    RECT rect;
    HRESULT d3dResult;

    text_height_ = 0;

    for (int i = 0; i < context.line_count; ++i) {
        const wchar_t* line = context.lines[i];
        int lineLength = context.lines_lengths[i];
        SIZE& lineSize = lines_sizes_[i];

        if (lineLength > 0) {
            rect.left = 0;
            rect.top = 0;
            rect.right = 0;
            rect.bottom = 0;

            d3dResult = font_->PreloadText (line, lineLength);

            INT textResult = font_->DrawTextW (
                sprite_, // sprite
                line, // text
                lineLength, // count,
                &rect, // rectangle
                DT_NOCLIP | DT_LEFT | DT_TOP | DT_SINGLELINE | DT_CALCRECT, // format
                0 // color
            );

            if (textResult == 0)
                textResult = font_height_ - 1;

            lineSize.cy = textResult + 1;
            lineSize.cx = rect.right + 1;

            text_height_ += lineSize.cy;
        } else {
            text_height_ += font_height_;
            lineSize.cx = 0;
            lineSize.cy = 0;
        }
    }
}

void FakeDirect3dDevice9::draw_subtitle ()
{
    if (is_device_lost_)
        return;


    const bbi::DllContext& context = LgVidContext;

    if (!context.show_subs) {
        bbi::ComHelper::release_and_null (font_);
        bbi::ComHelper::release_and_null (sprite_);
        return;
    }


    int line_count = context.line_count;

    if (line_count <= 0)
        return;


    HRESULT d3dResult = D3D_OK;

    if (sprite_ == 0) {
        d3dResult = d3dx9_funcs_->create_sprite (
            real_device_,
            &sprite_);
    }

    if (font_ == 0) {
        calculate_props ();

        if (font_height_ > 0) {
            d3dResult = d3dx9_funcs_->create_font (
                real_device_,
                font_height_, // height
                0, // width
                font_weight_, // weight
                0, // mipmap levels
                FALSE, // italic?
                DEFAULT_CHARSET, // charset
                OUT_TT_ONLY_PRECIS, // precision
                DEFAULT_QUALITY, // quality
                FF_DONTCARE | VARIABLE_PITCH, // pitch and family
                context.font_family, // typeface name
                &font_ // result
            );
        }
    }

    if (font_ == 0)
        return;


    if (old_sub_index_ != context.sub_index) {
        measure_text ();
        old_sub_index_ = context.sub_index;
    }

    if (sprite_ != 0)
        d3dResult = sprite_->Begin (D3DXSPRITE_ALPHABLEND);

    INT textResult = 0;

    RECT rect;


    int baseX = view_port_.X;
    int y = view_port_.Y + view_port_.Height - text_height_ - space_after_;


    for (int i = 0; i < line_count; ++i) {
        const wchar_t* line = context.lines[i];
        int lineLength = context.lines_lengths[i];
        const SIZE& lineSize = lines_sizes_[i];

        if ((lineLength > 0) && ((lineSize.cx > 0) && (lineSize.cy > 0))) {
            rect.left = baseX + (static_cast<LONG> (view_port_.Width) - lineSize.cx) / 2;
            rect.top = y;
            rect.right = rect.left + lineSize.cx;
            rect.bottom = rect.top + lineSize.cy;


            if ((shadow_offset_x_ != 0) && (shadow_offset_y_ != 0)) {
                rect.left += shadow_offset_x_;
                rect.top += shadow_offset_y_;

                textResult = font_->DrawTextW (
                    sprite_, // sprite
                    line, // text
                    lineLength, // count,
                    &rect, // rectangle
                    DT_NOCLIP | DT_LEFT | DT_TOP | DT_SINGLELINE, // format
                    context.shadow_color // color
                );

                rect.left -= shadow_offset_x_;
                rect.top -= shadow_offset_y_;
            }

            textResult = font_->DrawTextW (
                sprite_, // sprite
                line, // text
                lineLength, // count,
                &rect, // rectangle
                DT_NOCLIP | DT_LEFT | DT_TOP | DT_SINGLELINE, // format
                context.font_color // color
            );

            y += lineSize.cy;
        } else
            y += font_height_;
    }

    if (sprite_ != 0)
        d3dResult = sprite_->End ();
}
