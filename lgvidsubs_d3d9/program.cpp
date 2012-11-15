#include <memory>
#include <string>
#include <vector>

#include <d3dx9.h>
#include <tchar.h>

#include "bbi_dll_context.h"
#include "fake_direct3d9.h"


bbi::DllContext LgVidContext;


typedef std::basic_string<TCHAR> TString;

typedef IDirect3D9* (WINAPI* FP_DIRECT3DCREATE9) (UINT SDKVersion);


IDirect3D9* WINAPI Direct3DCreate9 (
    UINT SDKVersion)
{
    UINT nameLength = ::GetSystemDirectory (0, 0);

    if (nameLength == 0)
        return 0;

    std::auto_ptr<TCHAR> nameBuffer (new TCHAR[nameLength + 1]);

    if (nameBuffer.get () == 0)
        return 0;

    nameLength = ::GetSystemDirectory (nameBuffer.get (), nameLength);

    if (nameLength == 0)
        return 0;

    TString systemDir = nameBuffer.get ();
    TString d3d9LibraryName = systemDir + TEXT ("\\d3d9.dll");
    TString d3dX9LibraryName = systemDir + TEXT ("\\d3dx9_43.dll");

    IDirect3D9* fakeD3d9 = 0;
    IDirect3D9* realD3d9 = 0;
    HMODULE d3d9Module = ::LoadLibrary (d3d9LibraryName.c_str ());

    if (d3d9Module != 0) {
        FP_DIRECT3DCREATE9 d3dCreate9Func = reinterpret_cast<FP_DIRECT3DCREATE9> (
            ::GetProcAddress (d3d9Module, "Direct3DCreate9"));

        if (d3dCreate9Func != 0) {
            realD3d9 = d3dCreate9Func (SDKVersion);

            if (realD3d9 != 0)
                fakeD3d9 = new FakeDirect3d9 (d3d9Module, realD3d9, d3dX9LibraryName.c_str ());
        }
    }

    if (fakeD3d9 == 0) {
        if (realD3d9 != 0)
            realD3d9->Release ();

        if (d3d9Module != 0)
            ::FreeLibrary (d3d9Module);
    }

    return fakeD3d9;
}


BOOL WINAPI DllMain (
    HINSTANCE hinstDLL,
    DWORD fdwReason,
    LPVOID lpvReserved)
{
    return TRUE;
}
