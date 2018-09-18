#include <Windows.h>

#define DEFAULT_STORAGE_NEEDED 314572800
#define MAX_CONTIGUOUS_MEMORY_NEEDED 314572800

class BlueJadeApp {
public:
	//Members

	//Methods
	bool InitInstance();
	void CloseApp();
	void MainLoop();

private:
	/****
	Members
	****/

	//The mutex handle for this process
	HANDLE m_MutexHandle;

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
};