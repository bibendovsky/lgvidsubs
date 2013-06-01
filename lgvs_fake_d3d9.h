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


#ifndef LGVS_FAKE_D3D9_H
#define LGVS_FAKE_D3D9_H


#include "lgvs_d3dx9_funcs.h"


class FakeD3d9 : public IDirect3D9 {
public:
    FakeD3d9 (
        HMODULE real_d3d9_library,
        IDirect3D9* real_d3d9,
        LPCWSTR d3dx9_library_name);

    ~FakeD3d9 ();


    HRESULT STDMETHODCALLTYPE QueryInterface (
        REFIID riid,
        void** ppvObj);

    ULONG STDMETHODCALLTYPE AddRef ();

    ULONG STDMETHODCALLTYPE Release ();

    HRESULT STDMETHODCALLTYPE RegisterSoftwareDevice (
        void* pInitializeFunction);

    UINT STDMETHODCALLTYPE GetAdapterCount ();

    HRESULT STDMETHODCALLTYPE GetAdapterIdentifier (
        UINT Adapter,
        DWORD Flags,
        D3DADAPTER_IDENTIFIER9* pIdentifier);

    UINT STDMETHODCALLTYPE GetAdapterModeCount (
        UINT Adapter,
        D3DFORMAT Format);

    HRESULT STDMETHODCALLTYPE EnumAdapterModes (
        UINT Adapter,
        D3DFORMAT Format,
        UINT Mode,
        D3DDISPLAYMODE* pMode);

    HRESULT STDMETHODCALLTYPE GetAdapterDisplayMode (
        UINT Adapter,
        D3DDISPLAYMODE* pMode);

    HRESULT STDMETHODCALLTYPE CheckDeviceType (
        UINT Adapter,
        D3DDEVTYPE DevType,
        D3DFORMAT AdapterFormat,
        D3DFORMAT BackBufferFormat,
        BOOL bWindowed);

    HRESULT STDMETHODCALLTYPE CheckDeviceFormat (
        UINT Adapter,
        D3DDEVTYPE DeviceType,
        D3DFORMAT AdapterFormat,
        DWORD Usage,
        D3DRESOURCETYPE RType,
        D3DFORMAT CheckFormat);

    HRESULT STDMETHODCALLTYPE CheckDeviceMultiSampleType (
        UINT Adapter,
        D3DDEVTYPE DeviceType,
        D3DFORMAT SurfaceFormat,
        BOOL Windowed,
        D3DMULTISAMPLE_TYPE MultiSampleType,
        DWORD* pQualityLevels);

    HRESULT STDMETHODCALLTYPE CheckDepthStencilMatch (
        UINT Adapter,
        D3DDEVTYPE DeviceType,
        D3DFORMAT AdapterFormat,
        D3DFORMAT RenderTargetFormat,
        D3DFORMAT DepthStencilFormat);

    HRESULT STDMETHODCALLTYPE CheckDeviceFormatConversion (
        UINT Adapter,
        D3DDEVTYPE DeviceType,
        D3DFORMAT SourceFormat,
        D3DFORMAT TargetFormat);

    HRESULT STDMETHODCALLTYPE GetDeviceCaps (
        UINT Adapter,
        D3DDEVTYPE DeviceType,
        D3DCAPS9* pCaps);

    HMONITOR STDMETHODCALLTYPE GetAdapterMonitor (
        UINT Adapter);

    HRESULT STDMETHODCALLTYPE CreateDevice (
        UINT Adapter,
        D3DDEVTYPE DeviceType,
        HWND hFocusWindow,
        DWORD BehaviorFlags,
        D3DPRESENT_PARAMETERS* pPresentationParameters,
        IDirect3DDevice9** ppReturnedDeviceInterface);


private:
    HMODULE real_d3d9_module_;
    IDirect3D9* real_d3d9_;
    D3dx9Funcs* d3dx9_funcs_;

    FakeD3d9 (const FakeD3d9& that);

    FakeD3d9& operator = (const FakeD3d9& that);
}; // class FakeD3d9


#endif // LGVS_FAKE_D3D9_H
