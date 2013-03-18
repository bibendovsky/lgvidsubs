#include "fake_direct3d9.h"
#include "fake_direct3d_device9.h"


FakeDirect3d9::FakeDirect3d9 (HMODULE real_d3d9_library, IDirect3D9* real_d3d9,
    LPCTSTR d3dx9_library_name) :
        real_d3d9_module_ (real_d3d9_library),
        real_d3d9_ (real_d3d9),
        d3dx9_funcs_ (new D3dX9Funcs (d3dx9_library_name))
{
}

FakeDirect3d9::~FakeDirect3d9 ()
{
    delete d3dx9_funcs_;
    ::FreeLibrary (real_d3d9_module_);
}

HRESULT STDMETHODCALLTYPE FakeDirect3d9::QueryInterface (
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

ULONG STDMETHODCALLTYPE FakeDirect3d9::AddRef ()
{
    return real_d3d9_->AddRef ();
}

ULONG STDMETHODCALLTYPE FakeDirect3d9::Release ()
{
    ULONG result = real_d3d9_->Release ();

    if (result == 0)
        delete this;

    return result;
}

HRESULT STDMETHODCALLTYPE FakeDirect3d9::RegisterSoftwareDevice (
    void* pInitializeFunction)
{
    return real_d3d9_->RegisterSoftwareDevice (pInitializeFunction);
}

UINT STDMETHODCALLTYPE FakeDirect3d9::GetAdapterCount ()
{
    return real_d3d9_->GetAdapterCount ();
}

HRESULT STDMETHODCALLTYPE FakeDirect3d9::GetAdapterIdentifier (
    UINT Adapter,
    DWORD Flags,
    D3DADAPTER_IDENTIFIER9* pIdentifier)
{
    return real_d3d9_->GetAdapterIdentifier (Adapter, Flags, pIdentifier);
}

UINT STDMETHODCALLTYPE FakeDirect3d9::GetAdapterModeCount (
    UINT Adapter,
    D3DFORMAT Format)
{
    return real_d3d9_->GetAdapterModeCount (Adapter, Format);
}

HRESULT STDMETHODCALLTYPE FakeDirect3d9::EnumAdapterModes (
    UINT Adapter,
    D3DFORMAT Format,
    UINT Mode,
    D3DDISPLAYMODE* pMode)
{
    return real_d3d9_->EnumAdapterModes (Adapter, Format, Mode, pMode);
}

HRESULT STDMETHODCALLTYPE FakeDirect3d9::GetAdapterDisplayMode (
    UINT Adapter,
    D3DDISPLAYMODE* pMode)
{
    return real_d3d9_->GetAdapterDisplayMode (Adapter, pMode);
}

HRESULT STDMETHODCALLTYPE FakeDirect3d9::CheckDeviceType (
    UINT Adapter,
    D3DDEVTYPE DevType,
    D3DFORMAT AdapterFormat,
    D3DFORMAT BackBufferFormat,
    BOOL bWindowed)
{
    return real_d3d9_->CheckDeviceType (
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
    return real_d3d9_->CheckDeviceFormat (
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
    return real_d3d9_->CheckDeviceMultiSampleType (
        Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType, pQualityLevels);
}

HRESULT STDMETHODCALLTYPE FakeDirect3d9::CheckDepthStencilMatch (
    UINT Adapter,
    D3DDEVTYPE DeviceType,
    D3DFORMAT AdapterFormat,
    D3DFORMAT RenderTargetFormat,
    D3DFORMAT DepthStencilFormat)
{
    return real_d3d9_->CheckDepthStencilMatch (
        Adapter, DeviceType, AdapterFormat, RenderTargetFormat, DepthStencilFormat);
}

HRESULT STDMETHODCALLTYPE FakeDirect3d9::CheckDeviceFormatConversion (
    UINT Adapter,
    D3DDEVTYPE DeviceType,
    D3DFORMAT SourceFormat,
    D3DFORMAT TargetFormat)
{
    return real_d3d9_->CheckDeviceFormatConversion (
        Adapter, DeviceType, SourceFormat, TargetFormat);
}

HRESULT STDMETHODCALLTYPE FakeDirect3d9::GetDeviceCaps (
    UINT Adapter,
    D3DDEVTYPE DeviceType,
    D3DCAPS9* pCaps)
{
    return real_d3d9_->GetDeviceCaps (Adapter, DeviceType, pCaps);
}

HMONITOR STDMETHODCALLTYPE FakeDirect3d9::GetAdapterMonitor (
    UINT Adapter)
{
    return real_d3d9_->GetAdapterMonitor (Adapter);
}

HRESULT STDMETHODCALLTYPE FakeDirect3d9::CreateDevice (
    UINT Adapter,
    D3DDEVTYPE DeviceType,
    HWND hFocusWindow,
    DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS* pPresentationParameters,
    IDirect3DDevice9** ppReturnedDeviceInterface)
{
    IDirect3DDevice9* realDevice = NULL;
    FakeDirect3dDevice9* fakeDevice = NULL;

    HRESULT result = real_d3d9_->CreateDevice (
        Adapter, DeviceType, hFocusWindow, BehaviorFlags,
        pPresentationParameters, &realDevice);

    if (result == D3D_OK)
        fakeDevice = new FakeDirect3dDevice9 (realDevice, d3dx9_funcs_);

    if (fakeDevice != NULL)
        *ppReturnedDeviceInterface = fakeDevice;
    else {
        if (realDevice != NULL)
            realDevice->Release ();

        result = D3DERR_OUTOFVIDEOMEMORY;
    }

    return result;
}
