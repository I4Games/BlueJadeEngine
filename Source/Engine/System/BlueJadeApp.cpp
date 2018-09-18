#include "BlueJadeApp.h"
#include <iostream>
#include <direct.h>

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

	if (!CheckStorage(DEFAULT_STORAGE_NEEDED)) {
		cout << "Not enough space on disk. Exiting..." << endl;
	}
	else {
		cout << "Disk has enough space" << endl;
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

bool BlueJadeApp::CheckStorage(const DWORDLONG diskSpaceNeeded)
{
	//Get the address of the current drive
	int const drive = _getdrive();

	//Initialize a structure containing information about the current disk
	struct _diskfree_t diskfree;
	_getdiskfree(drive, &diskfree);

	//Calculate the amount of clusters needed in memory
	unsigned __int64 neededClusters;
	neededClusters = diskSpaceNeeded /
		(diskfree.sectors_per_cluster * diskfree.bytes_per_sector);

	//Determine if we have enough clusters
	if (diskfree.avail_clusters < neededClusters) {
		cout << "CheckStorage Failure: Not enough physical storage" << endl;
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
