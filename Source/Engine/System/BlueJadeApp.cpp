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
		return false;
	}
	else {
		cout << "Disk has enough space" << endl;
	}

	CheckMemory();

	return true;
}

bool BlueJadeApp::IsOnlyInstance(LPCTSTR gameTitle)
{
	//Create a mutex handle to point to the instance uniquely by name
	m_MutexHandle = CreateMutex(NULL, TRUE, gameTitle);

	//If the mutex fails to be created, point to the window of the
	//instance that is already running and return false
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

void BlueJadeApp::CheckMemory()
{
	//Get the global memory status
	MEMORYSTATUSEX status;
	GlobalMemoryStatusEx(&status);

	//Display total RAM available
	cout << "Total RAM: " << status.ullAvailPhys << " bytes available" << endl;

	//Display total VRAM available
	cout << "Total VRAM: " << status.ullAvailVirtual << " bytes available" << endl;

	//Determine contiguousness of memory (based on the max amount of VRAM the engine would use)
	char *buff = new char[MAX_CONTIGUOUS_MEMORY_NEEDED];
	if (buff) {
		delete[] buff;
		cout << "The system has " << MAX_CONTIGUOUS_MEMORY_NEEDED << " bytes of memory in a single block available" << endl;
	}
	else {
		cout << "The system does not have " << MAX_CONTIGUOUS_MEMORY_NEEDED << " bytes of memory in a single block available" << endl;
	}
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
