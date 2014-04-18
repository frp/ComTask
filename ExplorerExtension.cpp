#include "Dll.h"
#include "ExplorerExtension.h"
#include "FileItem.h"
#include "FileProcessor.h"
#include <list>
#include <cstdlib>
#include <fstream>
#include <codecvt>

static WCHAR const verbDisplayName[] = L"Calculate the Sum";
static WCHAR const c_szVerbName[] = L"Task.CalcTheSum";

ExplorerExtension::ExplorerExtension() : m_refCount(1), m_site(NULL), _hwnd(NULL), m_shellItemArray(NULL)
{
	DllAddRef();
}

IFACEMETHODIMP ExplorerExtension::QueryInterface(REFIID riid, void **ppv)
{
	static const QITAB qit[] =
	{
		QITABENT(ExplorerExtension, IExplorerCommand),
		QITABENT(ExplorerExtension, IInitializeCommand),     
		QITABENT(ExplorerExtension, IObjectWithSite),      
		{ 0 },
	};
	return QISearch(this, qit, riid, ppv);
}

// Reference counting: increase reference count
IFACEMETHODIMP_(ULONG) ExplorerExtension::AddRef()
{
	return ++m_refCount;
}

// Reference counting: decrease reference count
IFACEMETHODIMP_(ULONG) ExplorerExtension::Release()
{
	long mRef = --m_refCount;
	if (!mRef)
	{
		delete this;
	}
	return mRef;
}

// IExplorerCommand
// Get the title under which the item will be displayed in context menu
IFACEMETHODIMP ExplorerExtension::GetTitle(IShellItemArray * /* psiItemArray */, LPWSTR *ppszName)
{
	return SHStrDup(verbDisplayName, ppszName);
}

// Get the icon. In our case, menu item has no icon
IFACEMETHODIMP ExplorerExtension::GetIcon(IShellItemArray * /* psiItemArray */, LPWSTR *ppszIcon)
{
	*ppszIcon = NULL;
	return E_NOTIMPL;
}

// Get tool tip. In our case, menu item has no tooltip
IFACEMETHODIMP ExplorerExtension::GetToolTip(IShellItemArray * /* psiItemArray */, LPWSTR *ppszInfotip)
{
	// No tooltip
	*ppszInfotip = NULL;
	return E_NOTIMPL;
}

IFACEMETHODIMP ExplorerExtension::GetCanonicalName(GUID* pguidCommandName)
{
	*pguidCommandName = __uuidof(this);
	return S_OK;
}

// Get State (in our case, always enabled)
IFACEMETHODIMP ExplorerExtension::GetState(IShellItemArray * /* psiItemArray */, BOOL /*fOkToBeSlow*/, EXPCMDSTATE *pCmdState)
{
	*pCmdState = ECS_ENABLED;
	return S_OK;
}

// Get Flags (in our case, default)
IFACEMETHODIMP ExplorerExtension::GetFlags(EXPCMDFLAGS *pFlags)
{
	*pFlags = ECF_DEFAULT;
	return S_OK;
}

// Get subcommands (in our case, command has no subcommands
IFACEMETHODIMP ExplorerExtension::EnumSubCommands(IEnumExplorerCommand **ppEnum)
{
	*ppEnum = NULL;
	return E_NOTIMPL;
}

// IInitializeCommand
// This method is required, but in our case, does nothing
IFACEMETHODIMP ExplorerExtension::Initialize(PCWSTR, IPropertyBag *)
{
	return S_OK;
}

// IObjectWithSite
IFACEMETHODIMP ExplorerExtension::SetSite(IUnknown *punkSite)
{
	SetInterface(&m_site, punkSite);
	return S_OK;
}

IFACEMETHODIMP ExplorerExtension::GetSite(REFIID riid, void **ppv)
{
	*ppv = NULL;
	return m_site ? m_site->QueryInterface(riid, ppv) : E_FAIL;
}

int64_t getFileSize(const wchar_t * name)
{
	HANDLE hFile = CreateFileW(name, GENERIC_READ, FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) return -1;
	LARGE_INTEGER result;
	BOOL succeed = GetFileSizeEx(hFile, &result);
	CloseHandle(hFile);
	if (succeed)
		return result.QuadPart;
	else
		return -1;
}

DWORD ExplorerExtension::_ThreadProc()
{
	using namespace std;
	using namespace boost::posix_time;
	using namespace boost::gregorian;
	srand(static_cast<unsigned>(time(0)));

	wstring file_name = OrderedLogger::generateLogName();
	while (PathFileExists(file_name.c_str()))
		file_name = OrderedLogger::generateLogName();

	MessageBoxW(_hwnd, (L"Processing started. Result will be placed in the file \"" + file_name + L"\" in your user profile directory").c_str(), L"Calculate the Sum", MB_OK);

    IShellItemArray *psia;
    HRESULT hr = CoGetInterfaceAndReleaseStream(m_shellItemArray, IID_PPV_ARGS(&psia)); // Unmarshall data from Stream
    m_shellItemArray = NULL;
    if (SUCCEEDED(hr))
    {
        DWORD count;
        psia->GetCount(&count);

		list<FileItem> items;
		
        IShellItem2 *psi;
		for (DWORD i = 0; i < count && SUCCEEDED(hr); i++)
		{
			hr = GetItemAt(psia, i, IID_PPV_ARGS(&psi));
			if (SUCCEEDED(hr))
			{
				PWSTR shortName;
				hr = psi->GetDisplayName(SIGDN_PARENTRELATIVEPARSING, &shortName);
				if (SUCCEEDED(hr))
				{
					PWSTR fullName;
					hr = psi->GetDisplayName(SIGDN_FILESYSPATH, &fullName);
					if (SUCCEEDED(hr))
					{
						int64_t size = getFileSize(fullName);
						if (size != -1)
						{
							FILETIME ft;
							hr = psi->GetFileTime(PKEY_DateCreated, &ft);
							if (SUCCEEDED(hr))
							{
								SYSTEMTIME st;
								if (FileTimeToSystemTime(&ft, &st))
								{
									items.push_back({ shortName, fullName, size,
										ptime(date(st.wYear, st.wMonth, st.wDay),
										time_duration(st.wHour, st.wMinute, st.wSecond)) });
								}
								else
									hr = E_FAIL;
							}
						}
						else
							hr = E_FAIL;
						CoTaskMemFree(fullName);
					}
					CoTaskMemFree(shortName);
				}
				psi->Release();
			}
		}
        
        if (SUCCEEDED(hr))
        {
			wstring profile = _wgetenv(L"userprofile");
			if (profile[profile.size() - 1] != L'\\')
				profile += L'\\';
			wofstream logfile(profile + file_name);
			logfile.imbue(std::locale(logfile.getloc(), new std::codecvt_utf8_utf16<wchar_t>));
			OrderedLogger logger(OrderedLogger(logfile, true));
			FileProcessor processor(logger);
			processor.processFileList(items);
        }
        psia->Release();
    }

    return 0;
}

IFACEMETHODIMP ExplorerExtension::Invoke(IShellItemArray *psia, IBindCtx * /* pbc */)
{
    IUnknown_GetWindow(m_site, &_hwnd);

    HRESULT hr = CoMarshalInterThreadInterfaceInStream(__uuidof(psia), psia, &m_shellItemArray);
    if (SUCCEEDED(hr))
    {
        AddRef();
        if (!SHCreateThread(s_ThreadProc, this, CTF_COINIT_STA | CTF_PROCESS_REF, NULL))
        {
            Release();
        }
    }
    return S_OK;
}

static WCHAR const g_progId[] = L"*";

HRESULT ExplorerExtension::RegisterUnRegister(bool fRegister)
{
    RegisterExtension re(__uuidof(ExplorerExtension));

    HRESULT hr;
    if (fRegister)
    {
        hr = re.RegisterInProcServer(verbDisplayName, L"Apartment");
        if (SUCCEEDED(hr))
        {
            hr = re.RegisterExplorerCommandVerb(g_progId, c_szVerbName, verbDisplayName);
            if (SUCCEEDED(hr))
            {
                hr = re.RegisterVerbAttribute(g_progId, c_szVerbName, L"NeverDefault");
            }
        }
    }
    else
    {
        // best effort
        hr = re.UnRegisterVerb(g_progId, c_szVerbName);
        hr = re.UnRegisterObject();
    }
    return hr;
}

HRESULT ExplorerExtension::CreateInstance(REFIID riid, void **ppv)
{
    *ppv = NULL;
    ExplorerExtension *pVerb = new (std::nothrow) ExplorerExtension();
    HRESULT hr = pVerb ? S_OK : E_OUTOFMEMORY;
    if (SUCCEEDED(hr))
    {
        pVerb->QueryInterface(riid, ppv);
        pVerb->Release();
    }
    return hr;
}

ExplorerExtension::~ExplorerExtension()
{
	SafeRelease(&m_site);
	SafeRelease(&m_shellItemArray);
	DllRelease();
}

DWORD __stdcall ExplorerExtension::s_ThreadProc(void *pv)
{
	ExplorerExtension *pecv = reinterpret_cast<ExplorerExtension *>(pv);
	const DWORD ret = pecv->_ThreadProc();
	pecv->Release();
	return ret;
}