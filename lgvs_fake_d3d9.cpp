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


#include "lgvs_fake_d3d9.h"

#include "lgvs_fake_d3d9_device.h"


FakeD3d9::FakeD3d9 (HMODULE real_d3d9_library, IDirect3D9* real_d3d9,
    LPCWSTR d3dx9_library_name) :
        real_d3d9_module_ (real_d3d9_library),
        real_d3d9_ (real_d3d9),
        d3dx9_funcs_ (new D3dx9Funcs (d3dx9_library_name))
{
}

FakeD3d9::~FakeD3d9 ()
{
    delete d3dx9_funcs_;
    ::FreeLibrary (real_d3d9_module_);
}

HRESULT STDMETHODCALLTYPE FakeD3d9::QueryInterface (
    REFIID riid, void** ppvObj)
{
    if (ppvObj == NULL)
        return E_POINTER;

    if ((riid == IID_IUnknown) || (riid == IID_IDirect3D9)) {
        this->AddRef ();
        *ppvObj = this;
        return D3D_OK;
    }

    return real_d3d9_->QueryInterface (riid, ppvObj);
}

ULONG STDMETHODCALLTYPE FakeD3d9::AddRef ()
{
    return real_d3d9_->AddRef ();
}

ULONG STDMETHODCALLTYPE FakeD3d9::Release ()
{
    ULONG result = real_d3d9_->Release ();

    if (result == 0)
        delete this;

    return result;
}

HRESULT STDMETHODCALLTYPE FakeD3d9::RegisterSoftwareDevice (
    void* pInitializeFunction)
{
    return real_d3d9_->RegisterSoftwareDevice (pInitializeFunction);
}

UINT STDMETHODCALLTYPE FakeD3d9::GetAdapterCount ()
{
    return real_d3d9_->GetAdapterCount ();
}

HRESULT STDMETHODCALLTYPE FakeD3d9::GetAdapterIdentifier (
    UINT Adapter,
    DWORD Flags,
    D3DADAPTER_IDENTIFIER9* pIdentifier)
{
    return real_d3d9_->GetAdapterIdentifier (Adapter, Flags, pIdentifier);
}

UINT STDMETHODCALLTYPE FakeD3d9::GetAdapterModeCount (
    UINT Adapter,
    D3DFORMAT Format)
{
    return real_d3d9_->GetAdapterModeCount (Adapter, Format);
}

HRESULT STDMETHODCALLTYPE FakeD3d9::EnumAdapterModes (
    UINT Adapter,
    D3DFORMAT Format,
    UINT Mode,
    D3DDISPLAYMODE* pMode)
{
    return real_d3d9_->EnumAdapterModes (Adapter, Format, Mode, pMode);
}

HRESULT STDMETHODCALLTYPE FakeD3d9::GetAdapterDisplayMode (
    UINT Adapter,
    D3DDISPLAYMODE* pMode)
{
    return real_d3d9_->GetAdapterDisplayMode (Adapter, pMode);
}

HRESULT STDMETHODCALLTYPE FakeD3d9::CheckDeviceType (
    UINT Adapter,
    D3DDEVTYPE DevType,
    D3DFORMAT AdapterFormat,
    D3DFORMAT BackBufferFormat,
    BOOL bWindowed)
{
    return real_d3d9_->CheckDeviceType (
        Adapter, DevType, AdapterFormat, BackBufferFormat, bWindowed);
}

HRESULT STDMETHODCALLTYPE FakeD3d9::CheckDeviceFormat (
    UINT Adapter,
    D3DDEVTYPE DeviceType,
    D3DFORMAT AdapterFormat,
    DWORD Usage,
    D3DRESOURCETYPE RType,
    D3DFORMAT CheckFormat)
{
    return real_d3d9_->CheckDeviceFormat (
        Adapter, DeviceType, AdapterFormat, Usage, RType, CheckFormat);
}

HRESULT STDMETHODCALLTYPE FakeD3d9::CheckDeviceMultiSampleType (
    UINT Adapter,
    D3DDEVTYPE DeviceType,
    D3DFORMAT SurfaceFormat,
    BOOL Windowed,
    D3DMULTISAMPLE_TYPE MultiSampleType,
    DWORD* pQualityLevels)
{
    return real_d3d9_->CheckDeviceMultiSampleType (
        Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType, pQualityLevels);
}

HRESULT STDMETHODCALLTYPE FakeD3d9::CheckDepthStencilMatch (
    UINT Adapter,
    D3DDEVTYPE DeviceType,
    D3DFORMAT AdapterFormat,
    D3DFORMAT RenderTargetFormat,
    D3DFORMAT DepthStencilFormat)
{
    return real_d3d9_->CheckDepthStencilMatch (
        Adapter, DeviceType, AdapterFormat, RenderTargetFormat, DepthStencilFormat);
}

HRESULT STDMETHODCALLTYPE FakeD3d9::CheckDeviceFormatConversion (
    UINT Adapter,
    D3DDEVTYPE DeviceType,
    D3DFORMAT SourceFormat,
    D3DFORMAT TargetFormat)
{
    return real_d3d9_->CheckDeviceFormatConversion (
        Adapter, DeviceType, SourceFormat, TargetFormat);
}

HRESULT STDMETHODCALLTYPE FakeD3d9::GetDeviceCaps (
    UINT Adapter,
    D3DDEVTYPE DeviceType,
    D3DCAPS9* pCaps)
{
    return real_d3d9_->GetDeviceCaps (Adapter, DeviceType, pCaps);
}

HMONITOR STDMETHODCALLTYPE FakeD3d9::GetAdapterMonitor (
    UINT Adapter)
{
    return real_d3d9_->GetAdapterMonitor (Adapter);
}

HRESULT STDMETHODCALLTYPE FakeD3d9::CreateDevice (
    UINT Adapter,
    D3DDEVTYPE DeviceType,
    HWND hFocusWindow,
    DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS* pPresentationParameters,
    IDirect3DDevice9** ppReturnedDeviceInterface)
{
    IDirect3DDevice9* real_device = NULL;
    FakeD3d9Device* fake_device = NULL;

    HRESULT result = real_d3d9_->CreateDevice (
        Adapter, DeviceType, hFocusWindow, BehaviorFlags,
        pPresentationParameters, &real_device);

    if (result == D3D_OK)
        fake_device = new FakeD3d9Device (real_device, d3dx9_funcs_);

    if (fake_device != NULL)
        *ppReturnedDeviceInterface = fake_device;
    else {
        if (real_device != NULL)
            real_device->Release ();

        result = D3DERR_OUTOFVIDEOMEMORY;
    }

    return result;
}
