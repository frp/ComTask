#include "Dll.h"
#include <atomic>

class ExplorerExtension : public IExplorerCommand,
	public IInitializeCommand,
	public IObjectWithSite
{
public:
	ExplorerExtension();

	// IUnknown
	IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv);
	IFACEMETHODIMP_(ULONG) AddRef();
	IFACEMETHODIMP_(ULONG) Release();

	// IExplorerCommand
	IFACEMETHODIMP GetTitle(IShellItemArray * /* psiItemArray */, LPWSTR *ppszName);
	IFACEMETHODIMP GetIcon(IShellItemArray * /* psiItemArray */, LPWSTR *ppszIcon);
	IFACEMETHODIMP GetToolTip(IShellItemArray * /* psiItemArray */, LPWSTR *ppszInfotip);
	IFACEMETHODIMP GetCanonicalName(GUID* pguidCommandName);

	IFACEMETHODIMP GetState(IShellItemArray * /* psiItemArray */, BOOL /*fOkToBeSlow*/, EXPCMDSTATE *pCmdState);
	IFACEMETHODIMP Invoke(IShellItemArray *psiItemArray, IBindCtx *pbc);
	IFACEMETHODIMP GetFlags(EXPCMDFLAGS *pFlags);
	IFACEMETHODIMP EnumSubCommands(IEnumExplorerCommand **ppEnum);

	// IInitializeCommand
	IFACEMETHODIMP Initialize(PCWSTR /* pszCommandName */, IPropertyBag * /* ppb */);

	// IObjectWithSite
	IFACEMETHODIMP SetSite(IUnknown *punkSite);
	IFACEMETHODIMP GetSite(REFIID riid, void **ppv);

	static HRESULT ExplorerExtension::CreateInstance(REFIID riid, void **ppv);
	static HRESULT ExplorerExtension::RegisterUnRegister(bool fRegister);

private:
	~ExplorerExtension();

	DWORD _ThreadProc();

	static DWORD __stdcall s_ThreadProc(void *pv);

	std::atomic<long> m_refCount;
	IUnknown * m_site;
	HWND _hwnd;
	IStream * m_shellItemArray;
};