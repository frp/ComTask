#include "dll.h"
#include "ExplorerExtension.h"
#include <atomic>

std::atomic<long> g_refModule = 0;

// Handle the the DLL's module
HINSTANCE g_hInst = NULL;

// Standard DLL functions
STDAPI_(BOOL) DllMain(HINSTANCE hInstance, DWORD dwReason, void *)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        g_hInst = hInstance;
        DisableThreadLibraryCalls(hInstance);
    }
    return TRUE;
}

STDAPI DllCanUnloadNow()
{
    // Only allow the DLL to be unloaded after all outstanding references have been released
    return (g_refModule == 0) ? S_OK : S_FALSE;
}

void DllAddRef()
{
    g_refModule++;
}

void DllRelease()
{
    g_refModule--;
}

class CClassFactory : public IClassFactory
{
public:
    static HRESULT CreateInstance(REFCLSID clsid, /*const CLASS_OBJECT_INIT *pClassObjectInits, size_t cClassObjectInits,*/ REFIID riid, void **ppv)
    {
        *ppv = NULL;
        HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;
		if (clsid == __uuidof(ExplorerExtension))
		{
			IClassFactory *pClassFactory = new (std::nothrow) CClassFactory();
			hr = pClassFactory ? S_OK : E_OUTOFMEMORY;
			if (SUCCEEDED(hr))
			{
				hr = pClassFactory->QueryInterface(riid, ppv);
				pClassFactory->Release();
			}
		}
        return hr;
    }

    CClassFactory() : m_refCount(1)
    {
        DllAddRef();
    }

    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void ** ppv)
    {
        static const QITAB qit[] =
        {
            QITABENT(CClassFactory, IClassFactory),
            { 0 }
        };
        return QISearch(this, qit, riid, ppv);
    }

    IFACEMETHODIMP_(ULONG) AddRef()
    {
        return ++m_refCount;
    }

    IFACEMETHODIMP_(ULONG) Release()
    {
        long cRef = --m_refCount;
        if (cRef == 0)
        {
            delete this;
        }
        return cRef;
    }

    // IClassFactory
    IFACEMETHODIMP CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv)
    {
		return punkOuter ? CLASS_E_NOAGGREGATION : ExplorerExtension::CreateInstance(riid, ppv);
    }

    IFACEMETHODIMP LockServer(BOOL fLock)
    {
        if (fLock)
        {
            DllAddRef();
        }
        else
        {
            DllRelease();
        }
        return S_OK;
    }

private:
    ~CClassFactory()
    {
        DllRelease();
    }

    std::atomic<long> m_refCount;
};


STDAPI DllGetClassObject(REFCLSID clsid, REFIID riid, void **ppv)
{
    return CClassFactory::CreateInstance(clsid, /*c_rgClassObjectInit, ARRAYSIZE(c_rgClassObjectInit),*/ riid, ppv);
}

HRESULT RegisterUnregister(bool fRegister)
{
    return ExplorerExtension::RegisterUnRegister(fRegister);
}

STDAPI DllRegisterServer()
{
    return RegisterUnregister(true);
}

STDAPI DllUnregisterServer()
{
    return RegisterUnregister(false);
}
