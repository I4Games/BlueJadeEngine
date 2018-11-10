#include <windows.h>
#include <tchar.h>

#define DEFAULT_STORAGE_NEEDED 314572800
#define MAX_CONTIGUOUS_MEMORY_NEEDED 314572800

static TCHAR szWindowClass[] = _T("BlueJadeApp");
static TCHAR szTitle[] = _T("Blue Jade Engine Game");
static TCHAR windowMessage[] = _T("Hello World!");

class BlueJadeApp {
public:
	//Members

	//Methods
	bool InitInstance(HINSTANCE hInstance, int nCmdShow);
	void CloseApp();
	int MainLoop();

	//Window message handler
	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

private:
	/****
	Members
	****/

	//The mutex handle for this process
	HANDLE m_MutexHandle;

	//The current hinstance
	HINSTANCE m_hInst;

	/****
	Methods
	****/

	//Check if the current instance is the only one running
	bool IsOnlyInstance(LPCTSTR gameTitle);

	//Check if we have a certain amount of space on disk
	bool CheckStorage(const DWORDLONG diskSpaceNeeded);

	//Check RAM and VRAM, as well as how contiguous the VRAM is
	void CheckMemory();

	//Return cpu speed
	DWORD GetCPUSpeed();

	//Return cpu architecture
	LPCTSTR GetCPUArchitecture();

	//Create a new window
	bool InitializeWindow(HINSTANCE hInstance, int nCmdShow);

	
};