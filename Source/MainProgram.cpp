#include "Engine/System/BlueJadeApp.h"
#include <windows.h>  
#include <stdlib.h>  
#include <string.h>  
#include <tchar.h>  
 
BlueJadeApp* mGameApp = nullptr;

HINSTANCE hInst;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_ HINSTANCE hPrevInstance,
	_In_ LPSTR     lpCmdLine,
	_In_ int       nCmdShow
)
{
	mGameApp = new BlueJadeApp();

	if (!mGameApp->InitInstance(hInstance, nCmdShow)) {
		mGameApp->CloseApp();
		return 1;
	}

	return mGameApp->MainLoop();
}