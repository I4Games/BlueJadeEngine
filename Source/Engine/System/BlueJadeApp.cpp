#include "BlueJadeApp.h"
#include <iostream>
#include <string>

using namespace std;

bool BlueJadeApp::InitInstance()
{
	if (!IsOnlyInstance("BlueJadeWindow")) {
		cout << "Another instance of the game is running. Exiting..." << endl;
		return false;
	}
	else {
		cout << "No other instance of the game is running" << endl;
	}

	return true;
}

bool BlueJadeApp::IsOnlyInstance(LPCTSTR gameTitle)
{
	m_MutexHandle = CreateMutex(NULL, TRUE, gameTitle);

	if (GetLastError() != ERROR_SUCCESS) {
		HWND hWnd = FindWindow(gameTitle, NULL);
		if (hWnd) {
			ShowWindow(hWnd, SW_SHOWNORMAL);
			SetFocus(hWnd);
			SetForegroundWindow(hWnd);
			SetActiveWindow(hWnd);
		}
		return false;
	}

	return true;
}

void BlueJadeApp::CloseApp()
{
	if (m_MutexHandle) {
		ReleaseMutex(m_MutexHandle);
		CloseHandle(m_MutexHandle);
	}
}

void BlueJadeApp::MainLoop() {
	Sleep(10000);
}
