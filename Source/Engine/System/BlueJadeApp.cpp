#include "BlueJadeApp.h"
#include <iostream>
#include <direct.h>
#include <string.h>
#include <tchar.h>
#include <stdlib.h>
#include <windows.h>

using namespace std;

bool BlueJadeApp::InitInstance(HINSTANCE hInstance, int nCmdShow)
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

	if (!InitializeWindow(hInstance, nCmdShow)) {
		return false;
	}

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

LRESULT CALLBACK BlueJadeApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);

		TextOut(hdc,
			5, 5,
			windowMessage, _tcslen(windowMessage));

		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_LBUTTONDOWN:
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}

	return 0;
}

bool BlueJadeApp::InitializeWindow(HINSTANCE hInstance, int nCmdShow) {
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = this->WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));

	if (!RegisterClassEx(&wcex))
	{
		MessageBox(NULL,
			_T("Call to RegisterClassEx failed!"),
			_T("Win32 Guided Tour"),
			NULL);

		return false;
	}

	m_hInst = hInstance;

	HWND hWnd = CreateWindow(
		szWindowClass,
		szTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		500, 100,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (!hWnd)
	{
		MessageBox(NULL,
			_T("Call to CreateWindow failed!"),
			_T("Win32 Guided Tour"),
			NULL);

		return false;
	}

	ShowWindow(hWnd,
		nCmdShow);
	UpdateWindow(hWnd);

	return true;
}

void BlueJadeApp::CloseApp()
{
	if (m_MutexHandle) {
		ReleaseMutex(m_MutexHandle);
		CloseHandle(m_MutexHandle);
	}
}

int BlueJadeApp::MainLoop() {
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}
