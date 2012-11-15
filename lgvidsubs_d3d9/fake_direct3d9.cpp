#include "fake_direct3d9.h"
#include "fake_direct3d_device9.h"


FakeDirect3d9::FakeDirect3d9 (
    HMODULE realD3d9Library,
    IDirect3D9* realD3d9,
    LPCTSTR d3dX9LibraryName) :
        mRealD3d9Module (realD3d9Library),
        mRealD3d9 (realD3d9),
        mD3dX9Funcs (new D3dX9Funcs (d3dX9LibraryName))
{
}

FakeDirect3d9::~FakeDirect3d9 ()
{
    delete mD3dX9Funcs;
    ::FreeLibrary (mRealD3d9Module);
}

HRESULT STDMETHODCALLTYPE FakeDirect3d9::QueryInterface (
    REFIID riid,
    void** ppvObj)
{
    if (ppvObj == 0)
        return E_POINTER;

    if ((riid == IID_IUnknown) ||
        (riid == IID_IDirect3D9))
    {
        this->AddRef ();
        *ppvObj = this;
        return D3D_OK;
    }

    return mRealD3d9->QueryInterface (riid, ppvObj);
}

ULONG STDMETHODCALLTYPE FakeDirect3d9::AddRef ()
{
    return mRealD3d9->AddRef ();
}

ULONG STDMETHODCALLTYPE FakeDirect3d9::Release ()
{
    ULONG result = mRealD3d9->Release ();

    if (result == 0)
        delete this;

    return result;
}

HRESULT STDMETHODCALLTYPE FakeDirect3d9::RegisterSoftwareDevice (
    void* pInitializeFunction)
{
    return mRealD3d9->RegisterSoftwareDevice (pInitializeFunction);
}

UINT STDMETHODCALLTYPE FakeDirect3d9::GetAdapterCount ()
{
    return mRealD3d9->GetAdapterCount ();
}

HRESULT STDMETHODCALLTYPE FakeDirect3d9::GetAdapterIdentifier (
    UINT Adapter,
    DWORD Flags,
    D3DADAPTER_IDENTIFIER9* pIdentifier)
{
    return mRealD3d9->GetAdapterIdentifier (Adapter, Flags, pIdentifier);
}

UINT STDMETHODCALLTYPE FakeDirect3d9::GetAdapterModeCount (
    UINT Adapter,
    D3DFORMAT Format)
{
    return mRealD3d9->GetAdapterModeCount (Adapter, Format);
}

HRESULT STDMETHODCALLTYPE FakeDirect3d9::EnumAdapterModes (
    UINT Adapter,
    D3DFORMAT Format,
    UINT Mode,
    D3DDISPLAYMODE* pMode)
{
    return mRealD3d9->EnumAdapterModes (Adapter, Format, Mode, pMode);
}

HRESULT STDMETHODCALLTYPE FakeDirect3d9::GetAdapterDisplayMode (
    UINT Adapter,
    D3DDISPLAYMODE* pMode)
{
    return mRealD3d9->GetAdapterDisplayMode (Adapter, pMode);
}

HRESULT STDMETHODCALLTYPE FakeDirect3d9::CheckDeviceType (
    UINT Adapter,
    D3DDEVTYPE DevType,
    D3DFORMAT AdapterFormat,
    D3DFORMAT BackBufferFormat,
    BOOL bWindowed)
{
    return mRealD3d9->CheckDeviceType (
        Adapter, DevType, AdapterFormat, BackBufferFormat, bWindowed);
}

HRESULT STDMETHODCALLTYPE FakeDirect3d9::CheckDeviceFormat (
    UINT Adapter,
    D3DDEVTYPE DeviceType,
    D3DFORMAT AdapterFormat,
    DWORD Usage,
    D3DRESOURCETYPE RType,
    D3DFORMAT CheckFormat)
{
    return mRealD3d9->CheckDeviceFormat (
        Adapter, DeviceType, AdapterFormat, Usage, RType, CheckFormat);
}

HRESULT STDMETHODCALLTYPE FakeDirect3d9::CheckDeviceMultiSampleType (
    UINT Adapter,
    D3DDEVTYPE DeviceType,
    D3DFORMAT SurfaceFormat,
    BOOL Windowed,
    D3DMULTISAMPLE_TYPE MultiSampleType,
    DWORD* pQualityLevels)
{
    return mRealD3d9->CheckDeviceMultiSampleType (
        Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType, pQualityLevels);
}

HRESULT STDMETHODCALLTYPE FakeDirect3d9::CheckDepthStencilMatch (
    UINT Adapter,
    D3DDEVTYPE DeviceType,
    D3DFORMAT AdapterFormat,
    D3DFORMAT RenderTargetFormat,
    D3DFORMAT DepthStencilFormat)
{
    return mRealD3d9->CheckDepthStencilMatch (
        Adapter, DeviceType, AdapterFormat, RenderTargetFormat, DepthStencilFormat);
}

HRESULT STDMETHODCALLTYPE FakeDirect3d9::CheckDeviceFormatConversion (
    UINT Adapter,
    D3DDEVTYPE DeviceType,
    D3DFORMAT SourceFormat,
    D3DFORMAT TargetFormat)
{
    return mRealD3d9->CheckDeviceFormatConversion (
        Adapter, DeviceType, SourceFormat, TargetFormat);
}

HRESULT STDMETHODCALLTYPE FakeDirect3d9::GetDeviceCaps (
    UINT Adapter,
    D3DDEVTYPE DeviceType,
    D3DCAPS9* pCaps)
{
    return mRealD3d9->GetDeviceCaps (Adapter, DeviceType, pCaps);
}

HMONITOR STDMETHODCALLTYPE FakeDirect3d9::GetAdapterMonitor (
    UINT Adapter)
{
    return mRealD3d9->GetAdapterMonitor (Adapter);
}

HRESULT STDMETHODCALLTYPE FakeDirect3d9::CreateDevice (
    UINT Adapter,
    D3DDEVTYPE DeviceType,
    HWND hFocusWindow,
    DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS* pPresentationParameters,
    IDirect3DDevice9** ppReturnedDeviceInterface)
{
    IDirect3DDevice9* realDevice = 0;
    FakeDirect3dDevice9* fakeDevice = 0;

    HRESULT result = mRealD3d9->CreateDevice (
        Adapter, DeviceType, hFocusWindow, BehaviorFlags,
        pPresentationParameters, &realDevice);

    if (result == D3D_OK)
        fakeDevice = new FakeDirect3dDevice9 (realDevice, mD3dX9Funcs);

    if (fakeDevice != 0)
        *ppReturnedDeviceInterface = fakeDevice;
    else {
        if (realDevice != 0)
            realDevice->Release ();

        result = D3DERR_OUTOFVIDEOMEMORY;
    }

    return result;
}
