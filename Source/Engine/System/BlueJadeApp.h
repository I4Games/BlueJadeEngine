#include <Windows.h>

class BlueJadeApp {
public:
	//Members

	//Methods
	bool InitInstance();
	void CloseApp();
	void MainLoop();

private:
	//Members
	HANDLE m_MutexHandle;

	//Methods
	bool IsOnlyInstance(LPCTSTR gameTitle);
};