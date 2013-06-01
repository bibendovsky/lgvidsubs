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


#include "lgvs_fake_d3d9_device.h"

#include "lgvs_com_helper.h"
#include "lgvs_dll_context.h"
#include "lgvs_lgvid_d3d9.h"
#include "lgvs_math.h"


FakeD3d9Device::FakeD3d9Device (
    IDirect3DDevice9* real_device,
    D3dx9Funcs* d3dx9_funcs) :
        real_device_ (real_device),
        is_device_lost_ (false),
        d3dx9_funcs_ (d3dx9_funcs),
        font_ (NULL),
        sprite_ (NULL),
        font_height_ (0),
        font_weight_ (0),
        text_height_ (0),
        space_after_ (0),
        shadow_offset_x_ (0),
        shadow_offset_y_ (0),
        lines_sizes_ ()
{
    lines_sizes_.reserve (16);
}

FakeD3d9Device::~FakeD3d9Device ()
{
    lgvs::ComHelper::release_and_null (font_);
    lgvs::ComHelper::release_and_null (sprite_);
}


HRESULT STDMETHODCALLTYPE FakeD3d9Device::QueryInterface (
    REFIID riid, void** ppvObj)
{
    if (ppvObj == NULL)
        return E_POINTER;

    if (riid == IID_IUnknown || riid == IID_IDirect3DDevice9) {
        *ppvObj = this;
        return D3D_OK;
    }

    return real_device_->QueryInterface (riid, ppvObj);
}

ULONG STDMETHODCALLTYPE FakeD3d9Device::AddRef ()
{
    return real_device_->AddRef ();
}

ULONG STDMETHODCALLTYPE FakeD3d9Device::Release ()
{
    ULONG result = real_device_->Release ();

    if (result == 0)
        delete this;

    return result;
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::TestCooperativeLevel ()
{
    HRESULT result = real_device_->TestCooperativeLevel ();

    is_device_lost_ = (result != D3D_OK);

    if (is_device_lost_) {
        HRESULT d3d_result = D3D_OK;

        if (sprite_ != NULL)
            d3d_result = sprite_->OnLostDevice ();

        if (font_ != NULL)
            d3d_result = font_->OnLostDevice ();
    }

    return result;
}

UINT STDMETHODCALLTYPE FakeD3d9Device::GetAvailableTextureMem ()
{
    return real_device_->GetAvailableTextureMem ();
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::EvictManagedResources ()
{
    return real_device_->EvictManagedResources ();
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::GetDirect3D (
    IDirect3D9** ppD3D9)
{
    return real_device_->GetDirect3D (ppD3D9);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::GetDeviceCaps (
    D3DCAPS9* pCaps)
{
    return real_device_->GetDeviceCaps (pCaps);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::GetDisplayMode (
    UINT iSwapChain,
    D3DDISPLAYMODE* pMode)
{
    return real_device_->GetDisplayMode (iSwapChain, pMode);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::GetCreationParameters (
    D3DDEVICE_CREATION_PARAMETERS *pParameters)
{
    return real_device_->GetCreationParameters (pParameters);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::SetCursorProperties (
    UINT XHotSpot,
    UINT YHotSpot,
    IDirect3DSurface9* pCursorBitmap)
{
    return real_device_->SetCursorProperties (XHotSpot, YHotSpot, pCursorBitmap);
}

void STDMETHODCALLTYPE FakeD3d9Device::SetCursorPosition (
    int X,
    int Y,
    DWORD Flags)
{
    return real_device_->SetCursorPosition (X, Y, Flags);
}

BOOL STDMETHODCALLTYPE FakeD3d9Device::ShowCursor (
    BOOL bShow)
{
    return real_device_->ShowCursor (bShow);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::CreateAdditionalSwapChain (
    D3DPRESENT_PARAMETERS* pPresentationParameters,
    IDirect3DSwapChain9** pSwapChain)
{
    return real_device_->CreateAdditionalSwapChain (pPresentationParameters, pSwapChain);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::GetSwapChain (
    UINT iSwapChain,
    IDirect3DSwapChain9** pSwapChain)
{
    return real_device_->GetSwapChain (iSwapChain, pSwapChain);
}

UINT STDMETHODCALLTYPE FakeD3d9Device::GetNumberOfSwapChains ( )
{
    return real_device_->GetNumberOfSwapChains ();
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::Reset (
    D3DPRESENT_PARAMETERS* pPresentationParameters)
{
    HRESULT d3d_result = D3D_OK;

    if (lgvs::g_enable_subs) {
        if (sprite_ != NULL)
            d3d_result = sprite_->OnLostDevice ();

        if (font_ != NULL)
            d3d_result = font_->OnLostDevice ();
    }

    HRESULT result = real_device_->Reset (pPresentationParameters);

    if (lgvs::g_enable_subs)
        measure_text ();

    return result;
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::Present (
    CONST RECT* pSourceRect,
    CONST RECT* pDestRect,
    HWND hDestWindowOverride,
    CONST RGNDATA* pDirtyRegion)
{
    return real_device_->Present (
        pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::GetBackBuffer (
    UINT iSwapChain,
    UINT iBackBuffer,
    D3DBACKBUFFER_TYPE Type,
    IDirect3DSurface9** ppBackBuffer)
{
    return real_device_->GetBackBuffer (iSwapChain, iBackBuffer, Type, ppBackBuffer);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::GetRasterStatus (
    UINT iSwapChain,
    D3DRASTER_STATUS* pRasterStatus)
{
    return real_device_->GetRasterStatus (iSwapChain, pRasterStatus);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::SetDialogBoxMode (
    BOOL bEnableDialogs)
{
    return real_device_->SetDialogBoxMode (bEnableDialogs);
}

void STDMETHODCALLTYPE FakeD3d9Device::SetGammaRamp (
    UINT iSwapChain,
    DWORD Flags,
    CONST D3DGAMMARAMP* pRamp)
{
    return real_device_->SetGammaRamp (iSwapChain, Flags, pRamp);
}

void STDMETHODCALLTYPE FakeD3d9Device::GetGammaRamp (
    UINT iSwapChain,
    D3DGAMMARAMP* pRamp)
{
    return real_device_->GetGammaRamp (iSwapChain, pRamp);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::CreateTexture (
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

HRESULT STDMETHODCALLTYPE FakeD3d9Device::CreateVolumeTexture (
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

HRESULT STDMETHODCALLTYPE FakeD3d9Device::CreateCubeTexture (
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

HRESULT STDMETHODCALLTYPE FakeD3d9Device::CreateVertexBuffer (
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

HRESULT STDMETHODCALLTYPE FakeD3d9Device::CreateIndexBuffer (
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

HRESULT STDMETHODCALLTYPE FakeD3d9Device::CreateRenderTarget (
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

HRESULT STDMETHODCALLTYPE FakeD3d9Device::CreateDepthStencilSurface (
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

HRESULT STDMETHODCALLTYPE FakeD3d9Device::UpdateSurface (
    IDirect3DSurface9* pSourceSurface,
    CONST RECT* pSourceRect,
    IDirect3DSurface9* pDestinationSurface,
    CONST POINT* pDestPoint)
{
    return real_device_->UpdateSurface (
        pSourceSurface, pSourceRect, pDestinationSurface, pDestPoint);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::UpdateTexture (
    IDirect3DBaseTexture9* pSourceTexture,
    IDirect3DBaseTexture9* pDestinationTexture)
{
    return real_device_->UpdateTexture (pSourceTexture, pDestinationTexture);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::GetRenderTargetData (
    IDirect3DSurface9* pRenderTarget,
    IDirect3DSurface9* pDestSurface)
{
    return real_device_->GetRenderTargetData (pRenderTarget, pDestSurface);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::GetFrontBufferData (
    UINT iSwapChain,
    IDirect3DSurface9* pDestSurface)
{
    return real_device_->GetFrontBufferData (iSwapChain, pDestSurface);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::StretchRect (
    IDirect3DSurface9* pSourceSurface,
    CONST RECT* pSourceRect,
    IDirect3DSurface9* pDestSurface,
    CONST RECT* pDestRect,
    D3DTEXTUREFILTERTYPE Filter)
{
    return real_device_->StretchRect (
        pSourceSurface, pSourceRect, pDestSurface, pDestRect, Filter);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::ColorFill (
    IDirect3DSurface9* pSurface,
    CONST RECT* pRect,
    D3DCOLOR color)
{
    return real_device_->ColorFill (pSurface, pRect, color);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::CreateOffscreenPlainSurface (
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

HRESULT STDMETHODCALLTYPE FakeD3d9Device::SetRenderTarget (
    DWORD RenderTargetIndex,
    IDirect3DSurface9* pRenderTarget)
{
    return real_device_->SetRenderTarget (RenderTargetIndex, pRenderTarget);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::GetRenderTarget (
    DWORD RenderTargetIndex,
    IDirect3DSurface9** ppRenderTarget)
{
    return real_device_->GetRenderTarget (RenderTargetIndex, ppRenderTarget);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::SetDepthStencilSurface (
    IDirect3DSurface9* pNewZStencil)
{
    return real_device_->SetDepthStencilSurface (pNewZStencil);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::GetDepthStencilSurface (
    IDirect3DSurface9** ppZStencilSurface)
{
    return real_device_->GetDepthStencilSurface (ppZStencilSurface);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::BeginScene ()
{
    return real_device_->BeginScene ();
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::EndScene ()
{
    draw_subtitle ();

    return real_device_->EndScene ();
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::Clear (
    DWORD Count,
    CONST D3DRECT* pRects,
    DWORD Flags,
    D3DCOLOR Color,
    float Z,
    DWORD Stencil)
{
    return real_device_->Clear (Count, pRects, Flags, Color, Z, Stencil);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::SetTransform (
    D3DTRANSFORMSTATETYPE State,
    CONST D3DMATRIX* pMatrix)
{
    return real_device_->SetTransform (State, pMatrix);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::GetTransform (
    D3DTRANSFORMSTATETYPE State,
    D3DMATRIX* pMatrix)
{
    return real_device_->GetTransform (State, pMatrix);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::MultiplyTransform (
    D3DTRANSFORMSTATETYPE State,
    CONST D3DMATRIX* pMatrix)
{
    return real_device_->MultiplyTransform (State, pMatrix);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::SetViewport (
    CONST D3DVIEWPORT9* pViewport)
{
    return real_device_->SetViewport (pViewport);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::GetViewport (
    D3DVIEWPORT9* pViewport)
{
    return real_device_->GetViewport (pViewport);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::SetMaterial (
    CONST D3DMATERIAL9* pMaterial)
{
    return real_device_->SetMaterial (pMaterial);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::GetMaterial (
    D3DMATERIAL9* pMaterial)
{
    return real_device_->GetMaterial (pMaterial);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::SetLight (
    DWORD Index,
    CONST D3DLIGHT9* pLight)
{
    return real_device_->SetLight (Index, pLight);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::GetLight (
    DWORD Index,
    D3DLIGHT9* pLight)
{
    return real_device_->GetLight (Index, pLight);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::LightEnable (
    DWORD Index,
    BOOL Enable)
{
    return real_device_->LightEnable (Index, Enable);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::GetLightEnable (
    DWORD Index,
    BOOL* pEnable)
{
    return real_device_->GetLightEnable (Index, pEnable);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::SetClipPlane (
    DWORD Index,
    CONST float* pPlane)
{
    return real_device_->SetClipPlane (Index, pPlane);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::GetClipPlane (
    DWORD Index,
    float* pPlane)
{
    return real_device_->GetClipPlane (Index, pPlane);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::SetRenderState (
    D3DRENDERSTATETYPE State,
    DWORD Value)
{
    return real_device_->SetRenderState (State, Value);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::GetRenderState (
    D3DRENDERSTATETYPE State,
    DWORD* pValue)
{
    return real_device_->GetRenderState (State, pValue);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::CreateStateBlock (
    D3DSTATEBLOCKTYPE Type,
    IDirect3DStateBlock9** ppSB)
{
    return real_device_->CreateStateBlock (Type, ppSB);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::BeginStateBlock ( )
{
    return real_device_->BeginStateBlock ();
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::EndStateBlock (
    IDirect3DStateBlock9** ppSB)
{
    return real_device_->EndStateBlock (ppSB);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::SetClipStatus (
    CONST D3DCLIPSTATUS9* pClipStatus)
{
    return real_device_->SetClipStatus (pClipStatus);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::GetClipStatus (
    D3DCLIPSTATUS9* pClipStatus)
{
    return real_device_->GetClipStatus (pClipStatus);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::GetTexture (
    DWORD Stage,
    IDirect3DBaseTexture9** ppTexture)
{
    return real_device_->GetTexture (Stage, ppTexture);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::SetTexture (
    DWORD Stage,
    IDirect3DBaseTexture9* pTexture)
{
    return real_device_->SetTexture (Stage, pTexture);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::GetTextureStageState (
    DWORD Stage,
    D3DTEXTURESTAGESTATETYPE Type,
    DWORD* pValue)
{
    return real_device_->GetTextureStageState (Stage, Type, pValue);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::SetTextureStageState (
    DWORD Stage,
    D3DTEXTURESTAGESTATETYPE Type,
    DWORD Value)
{
    return real_device_->SetTextureStageState (Stage, Type, Value);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::GetSamplerState (
    DWORD Sampler,
    D3DSAMPLERSTATETYPE Type,
    DWORD* pValue)
{
    return real_device_->GetSamplerState (Sampler, Type, pValue);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::SetSamplerState (
    DWORD Sampler,
    D3DSAMPLERSTATETYPE Type,
    DWORD Value)
{
    return real_device_->SetSamplerState (Sampler, Type, Value);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::ValidateDevice (
    DWORD* pNumPasses)
{
    return real_device_->ValidateDevice (pNumPasses);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::SetPaletteEntries (
    UINT PaletteNumber,
    CONST PALETTEENTRY* pEntries)
{
    return real_device_->SetPaletteEntries (PaletteNumber, pEntries);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::GetPaletteEntries (
    UINT PaletteNumber,
    PALETTEENTRY* pEntries)
{
    return real_device_->GetPaletteEntries (PaletteNumber, pEntries);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::SetCurrentTexturePalette (
    UINT PaletteNumber)
{
    return real_device_->SetCurrentTexturePalette (PaletteNumber);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::GetCurrentTexturePalette (
    UINT *PaletteNumber)
{
    return real_device_->GetCurrentTexturePalette (PaletteNumber);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::SetScissorRect (
    CONST RECT* pRect)
{
    return real_device_->SetScissorRect (pRect);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::GetScissorRect (
    RECT* pRect)
{
    return real_device_->GetScissorRect (pRect);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::SetSoftwareVertexProcessing (
    BOOL bSoftware)
{
    return real_device_->SetSoftwareVertexProcessing (bSoftware);
}

BOOL STDMETHODCALLTYPE FakeD3d9Device::GetSoftwareVertexProcessing ()
{
    return real_device_->GetSoftwareVertexProcessing ();
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::SetNPatchMode (
    float nSegments)
{
    return real_device_->SetNPatchMode (nSegments);
}

float STDMETHODCALLTYPE FakeD3d9Device::GetNPatchMode ()
{
    return real_device_->GetNPatchMode ();
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::DrawPrimitive (
    D3DPRIMITIVETYPE PrimitiveType,
    UINT StartVertex,
    UINT PrimitiveCount)
{
    return real_device_->DrawPrimitive (PrimitiveType, StartVertex, PrimitiveCount);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::DrawIndexedPrimitive (
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

HRESULT STDMETHODCALLTYPE FakeD3d9Device::DrawPrimitiveUP (
    D3DPRIMITIVETYPE PrimitiveType,
    UINT PrimitiveCount,
    CONST void* pVertexStreamZeroData,
    UINT VertexStreamZeroStride)
{
    return real_device_->DrawPrimitiveUP (
        PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::DrawIndexedPrimitiveUP (
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

HRESULT STDMETHODCALLTYPE FakeD3d9Device::ProcessVertices (
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

HRESULT STDMETHODCALLTYPE FakeD3d9Device::CreateVertexDeclaration (
    CONST D3DVERTEXELEMENT9* pVertexElements,
    IDirect3DVertexDeclaration9** ppDecl)
{
    return real_device_->CreateVertexDeclaration (pVertexElements, ppDecl);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::SetVertexDeclaration (
    IDirect3DVertexDeclaration9* pDecl)
{
    return real_device_->SetVertexDeclaration (pDecl);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::GetVertexDeclaration (
    IDirect3DVertexDeclaration9** ppDecl)
{
    return real_device_->GetVertexDeclaration (ppDecl);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::SetFVF (
    DWORD FVF)
{
    return real_device_->SetFVF (FVF);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::GetFVF (
    DWORD* pFVF)
{
    return real_device_->GetFVF (pFVF);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::CreateVertexShader (
    CONST DWORD* pFunction,
    IDirect3DVertexShader9** ppShader)
{
    return real_device_->CreateVertexShader (pFunction, ppShader);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::SetVertexShader (
    IDirect3DVertexShader9* pShader)
{
    return real_device_->SetVertexShader (pShader);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::GetVertexShader (
    IDirect3DVertexShader9** ppShader)
{
    return real_device_->GetVertexShader (ppShader);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::SetVertexShaderConstantF (
    UINT StartRegister,
    CONST float* pConstantData,
    UINT Vector4fCount)
{
    return real_device_->SetVertexShaderConstantF (
        StartRegister, pConstantData, Vector4fCount);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::GetVertexShaderConstantF (
    UINT StartRegister,
    float* pConstantData,
    UINT Vector4fCount)
{
    return real_device_->GetVertexShaderConstantF (
        StartRegister, pConstantData, Vector4fCount);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::SetVertexShaderConstantI (
    UINT StartRegister,
    CONST int* pConstantData,
    UINT Vector4iCount)
{
    return real_device_->SetVertexShaderConstantI (
        StartRegister, pConstantData, Vector4iCount);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::GetVertexShaderConstantI (
    UINT StartRegister,
    int* pConstantData,
    UINT Vector4iCount)
{
    return real_device_->GetVertexShaderConstantI (
        StartRegister, pConstantData, Vector4iCount);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::SetVertexShaderConstantB (
    UINT StartRegister,
    CONST BOOL* pConstantData,
    UINT  BoolCount)
{
    return real_device_->SetVertexShaderConstantB (
        StartRegister, pConstantData, BoolCount);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::GetVertexShaderConstantB (
    UINT StartRegister,
    BOOL* pConstantData,
    UINT BoolCount)
{
    return real_device_->GetVertexShaderConstantB (
        StartRegister, pConstantData, BoolCount);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::SetStreamSource (
    UINT StreamNumber,
    IDirect3DVertexBuffer9* pStreamData,
    UINT OffsetInBytes,
    UINT Stride)
{
    return real_device_->SetStreamSource (
        StreamNumber, pStreamData, OffsetInBytes, Stride);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::GetStreamSource (
    UINT StreamNumber,
    IDirect3DVertexBuffer9** ppStreamData,
    UINT* pOffsetInBytes,
    UINT* pStride)
{
    return real_device_->GetStreamSource (
        StreamNumber, ppStreamData, pOffsetInBytes, pStride);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::SetStreamSourceFreq (
    UINT StreamNumber,
    UINT Setting)
{
    return real_device_->SetStreamSourceFreq (StreamNumber, Setting);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::GetStreamSourceFreq (
    UINT StreamNumber,
    UINT* pSetting)
{
    return real_device_->GetStreamSourceFreq (StreamNumber, pSetting);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::SetIndices (
    IDirect3DIndexBuffer9* pIndexData)
{
    return real_device_->SetIndices (pIndexData);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::GetIndices (
    IDirect3DIndexBuffer9** ppIndexData)
{
    return real_device_->GetIndices (ppIndexData);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::CreatePixelShader (
    CONST DWORD* pFunction,
    IDirect3DPixelShader9** ppShader)
{
    return real_device_->CreatePixelShader (pFunction, ppShader);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::SetPixelShader (
    IDirect3DPixelShader9* pShader)
{
    return real_device_->SetPixelShader (pShader);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::GetPixelShader (
    IDirect3DPixelShader9** ppShader)
{
    return real_device_->GetPixelShader (ppShader);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::SetPixelShaderConstantF (
    UINT StartRegister,
    CONST float* pConstantData,
    UINT Vector4fCount)
{
    return real_device_->SetPixelShaderConstantF (
        StartRegister, pConstantData, Vector4fCount);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::GetPixelShaderConstantF (
    UINT StartRegister,
    float* pConstantData,
    UINT Vector4fCount)
{
    return real_device_->GetPixelShaderConstantF (
        StartRegister, pConstantData, Vector4fCount);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::SetPixelShaderConstantI (
    UINT StartRegister,
    CONST int* pConstantData,
    UINT Vector4iCount)
{
    return real_device_->SetPixelShaderConstantI (
        StartRegister, pConstantData, Vector4iCount);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::GetPixelShaderConstantI (
    UINT StartRegister,
    int* pConstantData,
    UINT Vector4iCount)
{
    return real_device_->GetPixelShaderConstantI (
        StartRegister, pConstantData, Vector4iCount);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::SetPixelShaderConstantB (
    UINT StartRegister,
    CONST BOOL* pConstantData,
    UINT  BoolCount)
{
    return real_device_->SetPixelShaderConstantB (
        StartRegister, pConstantData, BoolCount);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::GetPixelShaderConstantB (
    UINT StartRegister,
    BOOL* pConstantData,
    UINT BoolCount)
{
    return real_device_->GetPixelShaderConstantB (
        StartRegister, pConstantData, BoolCount);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::DrawRectPatch (
    UINT Handle,
    CONST float* pNumSegs,
    CONST D3DRECTPATCH_INFO* pRectPatchInfo)
{
    return real_device_->DrawRectPatch (Handle, pNumSegs, pRectPatchInfo);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::DrawTriPatch (
    UINT Handle,
    CONST float* pNumSegs,
    CONST D3DTRIPATCH_INFO* pTriPatchInfo)
{
    return real_device_->DrawTriPatch (Handle, pNumSegs, pTriPatchInfo);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::DeletePatch (
    UINT Handle)
{
    return real_device_->DeletePatch (Handle);
}

HRESULT STDMETHODCALLTYPE FakeD3d9Device::CreateQuery (
    D3DQUERYTYPE Type,
    IDirect3DQuery9** ppQuery)
{
    return real_device_->CreateQuery (Type, ppQuery);
}

void FakeD3d9Device::calculate_props ()
{
    font_height_ = 0;
    text_height_ = 0;
    space_after_ = 0;

    if (!lgvs::g_enable_subs)
        return;


    HRESULT d3d_result;

    d3d_result = real_device_->GetViewport (&view_port_);

    if (d3d_result != D3D_OK)
        return;


    float font_height_f = lgvs::g_font_info.font_size;

    if (lgvs::g_font_info.font_size_in_percents)
        font_height_f *= view_port_.Height * 0.01F;

    font_height_ = static_cast<int> (font_height_f);
    font_weight_ = static_cast<int> (lgvs::g_font_info.font_weight);


    float space_after_f = lgvs::g_info.space_after;

    if (lgvs::g_info.space_after_in_percents)
        space_after_f *= view_port_.Height * 0.01F;

    if (space_after_f < 0.0F)
        space_after_f = 0.0F;

    space_after_ = static_cast<int> (space_after_f);


    float shadow_offset_xf = lgvs::g_font_info.shadow_offset_x;

    if (lgvs::g_font_info.shadow_offset_x_in_percents)
        shadow_offset_xf *= view_port_.Height * 0.01F;

    shadow_offset_x_ = static_cast<int> (shadow_offset_xf);


    float shadow_offset_y_f = lgvs::g_font_info.shadow_offset_y;

    if (lgvs::g_font_info.shadow_offset_y_in_percents)
        shadow_offset_y_f *= view_port_.Height * 0.01F;

    shadow_offset_y_ = static_cast<int> (shadow_offset_y_f);
}

void FakeD3d9Device::measure_text ()
{
    RECT rect;
    HRESULT d3d_result;

    text_height_ = 0;

    if (lgvs::g_sub_index < 0)
        return;

    if (lgvs::g_subs.empty ())
        return;

    const lgvs::Subtitle& subtitle = lgvs::g_subs[lgvs::g_sub_index];
    const lgvs::WStringList& lines = subtitle.lines;
    lines_sizes_.resize (lines.size ());

    for (size_t i = 0; i < lines.size (); ++i) {
        const std::wstring& line = lines[i];
        size_t line_length = line.size ();
        SIZE& line_size = lines_sizes_[i];

        if (line_length > 0) {
            rect.left = 0;
            rect.top = 0;
            rect.right = 0;
            rect.bottom = 0;

            d3d_result = font_->PreloadText (line.c_str (), line_length);

            INT text_result = font_->DrawTextW (
                sprite_, // sprite
                line.c_str (), // text
                line_length, // count,
                &rect, // rectangle
                DT_NOCLIP | DT_LEFT | DT_TOP | DT_SINGLELINE | DT_CALCRECT, // format
                0 // color
            );

            if (text_result == 0)
                text_result = font_height_ - 1;

            line_size.cy = text_result + 1;
            line_size.cx = rect.right + 1;

            text_height_ += line_size.cy;
        } else {
            text_height_ += font_height_;
            line_size.cx = 0;
            line_size.cy = 0;
        }
    }
}

void FakeD3d9Device::draw_subtitle ()
{
    if (is_device_lost_)
        return;

    if (!lgvs::g_enable_subs) {
        lgvs::ComHelper::release_and_null (font_);
        lgvs::ComHelper::release_and_null (sprite_);
        return;
    }

    if (lgvs::g_sub_index < 0)
        return;

    const lgvs::Subtitle& subtitle = lgvs::g_subs[lgvs::g_sub_index];

    size_t line_count = subtitle.lines.size ();

    if (line_count == 0)
        return;


    HRESULT d3d_result = D3D_OK;

    if (sprite_ == NULL) {
        d3d_result = d3dx9_funcs_->create_sprite (
            real_device_,
            &sprite_);
    }

    if (font_ == NULL) {
        calculate_props ();

        if (font_height_ > 0) {
            d3d_result = d3dx9_funcs_->create_font (
                real_device_,
                font_height_, // height
                0, // width
                font_weight_, // weight
                0, // mipmap levels
                FALSE, // italic?
                DEFAULT_CHARSET, // charset
                OUT_TT_PRECIS, // precision
                DEFAULT_QUALITY, // quality
                FF_DONTCARE | DEFAULT_PITCH, // pitch and family
                lgvs::g_font_info.font_family.c_str (), // typeface name
                &font_ // result
            );
        }
    }

    if (font_ == NULL)
        return;


    if (lgvs::g_old_sub_index != lgvs::g_sub_index) {
        measure_text ();
        lgvs::g_old_sub_index = lgvs::g_sub_index;
    }

    if (sprite_ != NULL)
        d3d_result = sprite_->Begin (D3DXSPRITE_ALPHABLEND);

    INT text_result = 0;

    RECT rect;


    int base_x = view_port_.X;
    int y = view_port_.Y + view_port_.Height - text_height_ - space_after_;
    const lgvs::WStringList& lines = subtitle.lines;

    for (size_t i = 0; i < line_count; ++i) {
        const std::wstring& line = lines[i];
        size_t line_length = line.size ();
        const SIZE& line_size = lines_sizes_[i];

        if ((line_length > 0) && ((line_size.cx > 0) && (line_size.cy > 0))) {
            rect.left = base_x + (static_cast<LONG> (view_port_.Width) - line_size.cx) / 2;
            rect.top = y;
            rect.right = rect.left + line_size.cx;
            rect.bottom = rect.top + line_size.cy;


            if (shadow_offset_x_ != 0 && shadow_offset_y_ != 0) {
                rect.left += shadow_offset_x_;
                rect.top += shadow_offset_y_;

                text_result = font_->DrawTextW (
                    sprite_, // sprite
                    line.c_str (), // text
                    line_length, // count,
                    &rect, // rectangle
                    DT_NOCLIP | DT_LEFT | DT_TOP | DT_SINGLELINE, // format
                    lgvs::g_font_info.shadow_color // color
                );

                rect.left -= shadow_offset_x_;
                rect.top -= shadow_offset_y_;
            }

            text_result = font_->DrawTextW (
                sprite_, // sprite
                line.c_str (), // text
                line_length, // count,
                &rect, // rectangle
                DT_NOCLIP | DT_LEFT | DT_TOP | DT_SINGLELINE, // format
                lgvs::g_font_info.font_color // color
            );

            y += line_size.cy;
        } else
            y += font_height_;
    }

    if (sprite_ != 0)
        d3d_result = sprite_->End ();
}
