#include "RegisterExtension.h"
#include <shlobj.h>
#include <shlwapi.h>
#include <strsafe.h>
#include <shobjidl.h>

#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "shlwapi.lib")     // link to this

// retrieve the HINSTANCE for the current DLL or EXE using this symbol that
// the linker provides for every module, avoids the need for a global HINSTANCE variable
// and provides access to this value for static libraries
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
__inline HINSTANCE GetModuleHINSTANCE() { return (HINSTANCE)&__ImageBase; }

RegisterExtension::RegisterExtension(REFCLSID clsid /* = CLSID_NULL */, HKEY hkeyRoot /* = HKEY_CURRENT_USER */) : _hkeyRoot(hkeyRoot), _fAssocChanged(false)
{
    SetHandlerCLSID(clsid);
    GetModuleFileName(GetModuleHINSTANCE(), _szModule, ARRAYSIZE(_szModule));
}

RegisterExtension::~RegisterExtension()
{
    if (_fAssocChanged)
    {
        // inform Explorer, et al that file association data has changed
        SHChangeNotify(SHCNE_ASSOCCHANGED, 0, 0, 0);
    }
}

void RegisterExtension::SetHandlerCLSID(REFCLSID clsid)
{
    _clsid = clsid;
    StringFromGUID2(_clsid, _szCLSID, ARRAYSIZE(_szCLSID));
}

void RegisterExtension::SetInstallScope(HKEY hkeyRoot)
{
    // must be HKEY_CURRENT_USER or HKEY_LOCAL_MACHINE
    _hkeyRoot = hkeyRoot;
}

HRESULT RegisterExtension::SetModule(PCWSTR pszModule)
{
    return StringCchCopy(_szModule, ARRAYSIZE(_szModule), pszModule);
}

HRESULT RegisterExtension::SetModule(HINSTANCE hinst)
{
    return GetModuleFileName(hinst, _szModule, ARRAYSIZE(_szModule)) ? S_OK : E_FAIL;
}

HRESULT RegisterExtension::_EnsureModule() const
{
    return _szModule[0] ? S_OK : E_FAIL;
}

HRESULT RegisterExtension::RegisterInProcServer(PCWSTR pszFriendlyName, PCWSTR pszThreadingModel) const
{
    HRESULT hr = _EnsureModule();
    if (SUCCEEDED(hr))
    {
        hr = RegSetKeyValuePrintf(_hkeyRoot, L"Software\\Classes\\CLSID\\%s", L"", pszFriendlyName, _szCLSID);
        if (SUCCEEDED(hr))
        {
            hr = RegSetKeyValuePrintf(_hkeyRoot, L"Software\\Classes\\CLSID\\%s\\InProcServer32", L"", _szModule, _szCLSID);
            if (SUCCEEDED(hr))
            {
                hr = RegSetKeyValuePrintf(_hkeyRoot, L"Software\\Classes\\CLSID\\%s\\InProcServer32", L"ThreadingModel", pszThreadingModel, _szCLSID);
            }
        }
    }
    return hr;
}

HRESULT RegisterExtension::UnRegisterObject() const
{
    // might have an AppID value, try that
    HRESULT hr = RegDeleteKeyPrintf(_hkeyRoot, L"Software\\Classes\\AppID\\%s", _szCLSID);
    if (SUCCEEDED(hr))
    {
        hr = RegDeleteKeyPrintf(_hkeyRoot, L"Software\\Classes\\CLSID\\%s", _szCLSID);
    }
    return hr;
}

// work around the missing "NeverDefault" feature for verbs on downlevel platforms
// these ProgID values should need special treatment to keep the verbs registered there
// from becoming default

bool RegisterExtension::_IsBaseClassProgID(PCWSTR pszProgID) const
{
    return !StrCmpIC(pszProgID, L"AllFileSystemObjects") ||
           !StrCmpIC(pszProgID, L"Directory") ||
           !StrCmpIC(pszProgID, L"*") ||
           StrStrI(pszProgID, L"SystemFileAssociations\\Directory.");   // SystemFileAssociations\Directory.* values
}

HRESULT RegisterExtension::_EnsureBaseProgIDVerbIsNone(PCWSTR pszProgID) const
{
    // putting the value of "none" that does not match any of the verbs under this key
    // avoids those verbs from becoming the default.
    return _IsBaseClassProgID(pszProgID) ?
        RegSetKeyValuePrintf(_hkeyRoot, L"Software\\Classes\\%s\\Shell", L"", L"none", pszProgID) :
        S_OK;
}

// must be an inproc handler registered here

HRESULT RegisterExtension::RegisterExplorerCommandVerb(PCWSTR pszProgID, PCWSTR pszVerb, PCWSTR pszVerbDisplayName) const
{
    UnRegisterVerb(pszProgID, pszVerb); // make sure no existing registration exists, ignore failure

    HRESULT hr = RegSetKeyValuePrintf(_hkeyRoot, L"Software\\Classes\\%s\\Shell\\%s",
        L"ExplorerCommandHandler", _szCLSID, pszProgID, pszVerb);
    if (SUCCEEDED(hr))
    {
        hr = _EnsureBaseProgIDVerbIsNone(pszProgID);

        hr = RegSetKeyValuePrintf(_hkeyRoot, L"Software\\Classes\\%s\\Shell\\%s",
            L"", pszVerbDisplayName, pszProgID, pszVerb);
    }
    return hr;
}

HRESULT RegisterExtension::UnRegisterVerb(PCWSTR pszProgID, PCWSTR pszVerb) const
{
    return RegDeleteKeyPrintf(_hkeyRoot, L"Software\\Classes\\%s\\Shell\\%s", pszProgID, pszVerb);
}

// NeverDefault
// LegacyDisable
// Extended
// OnlyInBrowserWindow
// ProgrammaticAccessOnly
// SeparatorBefore
// SeparatorAfter
// CheckSupportedTypes, used SupportedTypes that is a file type filter registered under AppPaths (I think)
HRESULT RegisterExtension::RegisterVerbAttribute(PCWSTR pszProgID, PCWSTR pszVerb, PCWSTR pszValueName) const
{
    return RegSetKeyValuePrintf(_hkeyRoot, L"Software\\Classes\\%s\\shell\\%s", pszValueName, L"", pszProgID, pszVerb);
}

// MUIVerb=@dll,-resid
// MultiSelectModel=Single|Player|Document
// Position=Bottom|Top
// DefaultAppliesTo=System.ItemName:"foo"
// HasLUAShield=System.ItemName:"bar"
// AppliesTo=System.ItemName:"foo"
HRESULT RegisterExtension::RegisterVerbAttribute(PCWSTR pszProgID, PCWSTR pszVerb, PCWSTR pszValueName, PCWSTR pszValue) const
{
    return RegSetKeyValuePrintf(_hkeyRoot, L"Software\\Classes\\%s\\shell\\%s", pszValueName, pszValue, pszProgID, pszVerb);
}

// BrowserFlags
// ExplorerFlags
// AttributeMask
// AttributeValue
// ImpliedSelectionModel
// SuppressionPolicy
HRESULT RegisterExtension::RegisterVerbAttribute(PCWSTR pszProgID, PCWSTR pszVerb, PCWSTR pszValueName, DWORD dwValue) const
{
    return RegSetKeyValuePrintf(_hkeyRoot, L"Software\\Classes\\%s\\shell\\%s", pszValueName, dwValue, pszProgID, pszVerb);
}

HRESULT RegisterExtension::RegSetKeyValuePrintf(HKEY hkey, PCWSTR pszKeyFormatString, PCWSTR pszValueName, PCWSTR pszValue, ...) const
{
    va_list argList;
    va_start(argList, pszValue);

    WCHAR szKeyName[512];
    HRESULT hr = StringCchVPrintf(szKeyName, ARRAYSIZE(szKeyName), pszKeyFormatString, argList);
    if (SUCCEEDED(hr))
    {
        hr = HRESULT_FROM_WIN32(RegSetKeyValueW(hkey, szKeyName, pszValueName, REG_SZ, pszValue,
            lstrlen(pszValue) * sizeof(*pszValue)));
    }

    va_end(argList);

    _UpdateAssocChanged(hr, pszKeyFormatString);
    return hr;
}

HRESULT RegisterExtension::RegSetKeyValuePrintf(HKEY hkey, PCWSTR pszKeyFormatString, PCWSTR pszValueName, DWORD dwValue, ...) const
{
    va_list argList;
    va_start(argList, dwValue);

    WCHAR szKeyName[512];
    HRESULT hr = StringCchVPrintf(szKeyName, ARRAYSIZE(szKeyName), pszKeyFormatString, argList);
    if (SUCCEEDED(hr))
    {
        hr = HRESULT_FROM_WIN32(RegSetKeyValueW(hkey, szKeyName, pszValueName, REG_DWORD, &dwValue, sizeof(dwValue)));
    }

    va_end(argList);

    _UpdateAssocChanged(hr, pszKeyFormatString);
    return hr;
}

HRESULT RegisterExtension::RegSetKeyValuePrintf(HKEY hkey, PCWSTR pszKeyFormatString, PCWSTR pszValueName, const unsigned char pc[], DWORD dwSize, ...) const
{
    va_list argList;
    va_start(argList, pc);

    WCHAR szKeyName[512];
    HRESULT hr = StringCchVPrintf(szKeyName, ARRAYSIZE(szKeyName), pszKeyFormatString, argList);
    if (SUCCEEDED(hr))
    {
        hr = HRESULT_FROM_WIN32(RegSetKeyValueW(hkey, szKeyName, pszValueName, REG_BINARY, pc, dwSize));
    }

    va_end(argList);

    _UpdateAssocChanged(hr, pszKeyFormatString);
    return hr;
}

HRESULT RegisterExtension::RegSetKeyValueBinaryPrintf(HKEY hkey, PCWSTR pszKeyFormatString, PCWSTR pszValueName, PCSTR pszBase64, ...) const
{
    va_list argList;
    va_start(argList, pszBase64);

    WCHAR szKeyName[512];
    HRESULT hr = StringCchVPrintf(szKeyName, ARRAYSIZE(szKeyName), pszKeyFormatString, argList);
    if (SUCCEEDED(hr))
    {
        DWORD dwDecodedImageSize, dwSkipChars, dwActualFormat;
        hr = CryptStringToBinaryA(pszBase64, NULL, CRYPT_STRING_BASE64, NULL,
            &dwDecodedImageSize, &dwSkipChars, &dwActualFormat) ? S_OK : E_FAIL;
        if (SUCCEEDED(hr))
        {
            BYTE *pbDecodedImage = (BYTE*)LocalAlloc(LPTR, dwDecodedImageSize);
            hr = pbDecodedImage ? S_OK : E_OUTOFMEMORY;
            if (SUCCEEDED(hr))
            {
                hr = CryptStringToBinaryA(pszBase64, lstrlenA(pszBase64), CRYPT_STRING_BASE64,
                    pbDecodedImage, &dwDecodedImageSize, &dwSkipChars, &dwActualFormat) ? S_OK : E_FAIL;
                if (SUCCEEDED(hr))
                {
                    hr = HRESULT_FROM_WIN32(RegSetKeyValueW(hkey, szKeyName, pszValueName, REG_BINARY, pbDecodedImage, dwDecodedImageSize));
                }
            }
        }
    }

    va_end(argList);

    _UpdateAssocChanged(hr, pszKeyFormatString);
    return hr;
}

__inline HRESULT MapNotFoundToSuccess(HRESULT hr)
{
    return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr ? S_OK : hr;
}

HRESULT RegisterExtension::RegDeleteKeyPrintf(HKEY hkey, PCWSTR pszKeyFormatString, ...) const
{
    va_list argList;
    va_start(argList, pszKeyFormatString);

    WCHAR szKeyName[512];
    HRESULT hr = StringCchVPrintf(szKeyName, ARRAYSIZE(szKeyName), pszKeyFormatString, argList);
    if (SUCCEEDED(hr))
    {
        hr = HRESULT_FROM_WIN32(RegDeleteTree(hkey, szKeyName));
    }

    va_end(argList);

    _UpdateAssocChanged(hr, pszKeyFormatString);
    return MapNotFoundToSuccess(hr);
}

HRESULT RegisterExtension::RegDeleteKeyValuePrintf(HKEY hkey, PCWSTR pszKeyFormatString, PCWSTR pszValue, ...) const
{
    va_list argList;
    va_start(argList, pszKeyFormatString);

    WCHAR szKeyName[512];
    HRESULT hr = StringCchVPrintf(szKeyName, ARRAYSIZE(szKeyName), pszKeyFormatString, argList);
    if (SUCCEEDED(hr))
    {
        hr = HRESULT_FROM_WIN32(RegDeleteKeyValueW(hkey, szKeyName, pszValue));
    }

    va_end(argList);

    _UpdateAssocChanged(hr, pszKeyFormatString);
    return MapNotFoundToSuccess(hr);
}

void RegisterExtension::_UpdateAssocChanged(HRESULT hr, PCWSTR pszKeyFormatString) const
{
    static const WCHAR c_szProgIDPrefix[] = L"Software\\Classes\\%s";
    if (SUCCEEDED(hr) && !_fAssocChanged &&
        (StrCmpNIC(pszKeyFormatString, c_szProgIDPrefix, ARRAYSIZE(c_szProgIDPrefix) - 1) == 0 ||
         StrStrI(pszKeyFormatString, L"PropertyHandlers") ||
         StrStrI(pszKeyFormatString, L"KindMap")))
    {
        const_cast<RegisterExtension*>(this)->_fAssocChanged = true;
    }
}
