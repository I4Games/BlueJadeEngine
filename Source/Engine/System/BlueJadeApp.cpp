#include "BlueJadeApp.h"
#include <iostream>
#include <direct.h>
#include <string.h>

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

	cout << "CPU Architecture: " << GetCPUArchitecture() << endl;
	cout << "CPU Speed: " << GetCPUSpeed() << " MHz" << endl;

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

DWORD BlueJadeApp::GetCPUSpeed()
{
	DWORD BufSize = sizeof(DWORD);
	DWORD dwMHz = 0;
	DWORD type = REG_DWORD;
	HKEY hKey;

	//Check the registry for the key that holds cpu speed
	long lError = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
		0,
		KEY_READ,
		&hKey);

	//If the key was foundm query it for the value we need
	if (lError == ERROR_SUCCESS) {
		RegQueryValueEx(hKey, "~MHz", NULL, NULL, (LPBYTE)&dwMHz, &BufSize);
	}

	return dwMHz;
}

LPCTSTR BlueJadeApp::GetCPUArchitecture()
{

	//Get system information
	SYSTEM_INFO system_info;
	GetNativeSystemInfo(&system_info);

	//Retrieve the architecure ID
	WORD arch_id = system_info.wProcessorArchitecture;

	//Return a value depending on the architecture ID
	switch (arch_id) {
		case PROCESSOR_ARCHITECTURE_AMD64:
			return "x64";
		case PROCESSOR_ARCHITECTURE_ARM:
			return "ARM";
		case PROCESSOR_ARCHITECTURE_ARM64:
			return "ARM64";
		case PROCESSOR_ARCHITECTURE_IA64:
			return "Intel Itanium-based";
		case PROCESSOR_ARCHITECTURE_INTEL:
			return "x86";
		case PROCESSOR_ARCHITECTURE_UNKNOWN:
			return "Unknown architecture";
		default:
			return "Invalid or unrecognized architecture code";
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
