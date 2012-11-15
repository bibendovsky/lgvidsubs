#ifndef FAKE_DIRECT3D9_H
#define FAKE_DIRECT3D9_H


#include "d3dx9_funcs.h"


class FakeDirect3d9 :
    public IDirect3D9 {
public:
    FakeDirect3d9 (
        HMODULE realD3d9Library,
        IDirect3D9* realD3d9,
        LPCTSTR d3dX9LibraryName);

    ~FakeDirect3d9 ();


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
    HMODULE mRealD3d9Module;
    IDirect3D9* mRealD3d9;
    D3dX9Funcs* mD3dX9Funcs;

    FakeDirect3d9 (
        const FakeDirect3d9& that);

    FakeDirect3d9& operator = (
        const FakeDirect3d9& that);
}; // class FakeDirect3d9


#endif // FAKE_DIRECT3D9_H
