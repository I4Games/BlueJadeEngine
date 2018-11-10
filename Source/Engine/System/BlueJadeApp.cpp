//mouse input:
//https://docs.microsoft.com/en-us/windows/desktop/inputdev/using-mouse-input

//keyboard input:
//https://docs.microsoft.com/en-us/windows/desktop/inputdev/using-keyboard-input

#include "BlueJadeApp.h"
#include <iostream>
#include <direct.h>
#include <string.h>
#include <tchar.h>
#include <stdlib.h>
#include <windows.h>

using namespace std;

#define BUFSIZE 65535 
#define SHIFTED 0x8000 

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


#define BUFSIZE 65535 
#define SHIFTED 0x8000 

LRESULT CALLBACK BlueJadeApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;	// required by BeginPaint 
	HDC hdc;

	static POINTS ptsBegin;        // beginning point 
	static POINTS ptsEnd;          // new endpoint 
	static POINTS ptsPrevEnd;      // previous endpoint 
	static BOOL fPrevLine = FALSE; // previous line flag 

	TEXTMETRIC tm;             // structure for text metrics 
	static DWORD dwCharX;      // average width of characters 
	static DWORD dwCharY;      // height of characters 
	static DWORD dwClientX;    // width of client area 
	static DWORD dwClientY;    // height of client area 
	static DWORD dwLineLen;    // line length 
	static DWORD dwLines;      // text lines in client area 
	static int nCaretPosX = 10; // horizontal position of caret 
	static int nCaretPosY = 2; // vertical position of caret 
	static int nCharWidth = 0; // width of a character 
	static int cch = 0;        // characters in buffer 
	static int nCurChar = 0;   // index of current character 
	static PTCHAR pchInputBuf; // input buffer 
	int i, j;                  // loop counters 
	int cCR = 0;               // count of carriage returns 
	int nCRIndex = 0;          // index of last carriage return 
	int nVirtKey;              // virtual-key code 
	TCHAR szBuf[128];          // temporary buffer 
	TCHAR ch;                  // current character 
	RECT rc;                   // output rectangle for DrawText 
	SIZE sz;                   // string dimensions 
	COLORREF crPrevText;       // previous text color 
	COLORREF crPrevBk;         // previous background color
	size_t * pcch;
	HRESULT hResult;

	switch (message)
	{

	case WM_LBUTTONDOWN:
		// Capture mouse input.
		SetCapture(hWnd);
		ptsBegin = MAKEPOINTS(lParam);
		return 0;
		break;
	case WM_MOUSEMOVE:
	
		// When moving the mouse, the user must hold down
		// the left mouse button to draw lines/ rectangle
		if (wParam & MK_LBUTTON)
		{
			//// Convert the current cursor coordinates to a
			//// POINTS structure, and then draw lines/ or a new rectangle.
			//ptsEnd = MAKEPOINTS(lParam);
			////draw a rectangle
			////Rectangle(hdc, ptsBegin.x, ptsBegin.y, ptsEnd.x, ptsEnd.y);

			//MoveToEx(hdc, ptsBegin.x, ptsBegin.y, (LPPOINT)NULL);
			//LineTo(hdc, ptsEnd.x, ptsEnd.y);
			//ReleaseDC(hWnd, hdc);
			// Retrieve a device context (DC) for the client area. 

			hdc = GetDC(hWnd);

			// The following function ensures that pixels of 
			// the previously drawn line are set to white and 
			// those of the new line are set to black. 

			SetROP2(hdc, R2_NOTXORPEN);

			// If a line was drawn during an earlier WM_MOUSEMOVE 
			// message, draw over it. This erases the line by 
			// setting the color of its pixels to white. 

			if (fPrevLine)
			{
				MoveToEx(hdc, ptsBegin.x, ptsBegin.y,
					(LPPOINT)NULL);
				LineTo(hdc, ptsPrevEnd.x, ptsPrevEnd.y);
			}

			// Convert the current cursor coordinates to a 
			// POINTS structure, and then draw a new line. 

			ptsEnd = MAKEPOINTS(lParam);
			MoveToEx(hdc, ptsBegin.x, ptsBegin.y, (LPPOINT)NULL);
			LineTo(hdc, ptsEnd.x, ptsEnd.y);

			//draw a rectangle
			//Rectangle(hdc, ptsBegin.x, ptsBegin.y, ptsEnd.x, ptsEnd.y);

			// Set the previous line flag, save the ending 
			// point of the new line, and then release the DC. 

			fPrevLine = TRUE;
			ptsPrevEnd = ptsEnd;
			ReleaseDC(hWnd, hdc);
		}
		break;
	case WM_LBUTTONUP:
		// The user has finished drawing the line. Reset the 
		// previous line flag, release the mouse cursor, and 
		// release the mouse capture. 
		fPrevLine = FALSE;
		ClipCursor(NULL);
		ReleaseCapture();
		return 0;

	case WM_CREATE:

		// Get the metrics of the current font. 
		hdc = GetDC(hWnd);
		GetTextMetrics(hdc, &tm);
		ReleaseDC(hWnd, hdc);

		// Save the average character width and height. 

		dwCharX = tm.tmAveCharWidth;
		dwCharY = tm.tmHeight;

		// Allocate a buffer to store keyboard input. 

		pchInputBuf = (LPTSTR)GlobalAlloc(GPTR,
			BUFSIZE * sizeof(TCHAR));
		return 0;

	case WM_SIZE:

		// Save the new width and height of the client area. 

		dwClientX = LOWORD(lParam);
		dwClientY = HIWORD(lParam);

		// Calculate the maximum width of a line and the 
		// maximum number of lines in the client area. 

		dwLineLen = dwClientX - dwCharX;
		dwLines = dwClientY / dwCharY;
		break;


	case WM_SETFOCUS:

		// Create, position, and display the caret when the 
		// window receives the keyboard focus. 

		CreateCaret(hWnd, (HBITMAP)1, 0, dwCharY);
		SetCaretPos(nCaretPosX, nCaretPosY * dwCharY);
		ShowCaret(hWnd);
		break;

	case WM_KILLFOCUS:

		// Hide and destroy the caret when the window loses the 
		// keyboard focus. 

		HideCaret(hWnd);
		DestroyCaret();
		break;

	case WM_CHAR:
		// check if current location is close enough to the
		// end of the buffer that a buffer overflow may
		// occur. If so, add null and display contents. 
		if (cch > BUFSIZE - 5)
		{
			pchInputBuf[cch] = 0x00;
			SendMessage(hWnd, WM_PAINT, 0, 0);
		}
		switch (wParam)
		{
		case 0x08:  // backspace 
		case 0x0A:  // linefeed 
		case 0x1B:  // escape 
			MessageBeep((UINT)-1);
			return 0;

		case 0x09:  // tab 

			// Convert tabs to four consecutive spaces. 

			for (i = 0; i < 4; i++)
				SendMessage(hWnd, WM_CHAR, 0x20, 0);
			return 0;

		case 0x0D:  // carriage return 

			// Record the carriage return and position the 
			// caret at the beginning of the new line.

			pchInputBuf[cch++] = 0x0D;
			nCaretPosX = 0;
			nCaretPosY += 1;
			break;

		default:    // displayable character 

			ch = (TCHAR)wParam;
			HideCaret(hWnd);

			// Retrieve the character's width and output 
			// the character. 

			hdc = GetDC(hWnd);
			GetCharWidth32(hdc, (UINT)wParam, (UINT)wParam,
				&nCharWidth);
			TextOut(hdc, nCaretPosX, nCaretPosY * dwCharY,
				&ch, 1);
		
			ReleaseDC(hWnd, hdc);

			// Store the character in the buffer.

			pchInputBuf[cch++] = ch;

			// Calculate the new horizontal position of the 
			// caret. If the position exceeds the maximum, 
			// insert a carriage return and move the caret 
			// to the beginning of the next line. 

			nCaretPosX += nCharWidth;
			if ((DWORD)nCaretPosX > dwLineLen)
			{
				nCaretPosX = 0;
				pchInputBuf[cch++] = 0x0D;
				++nCaretPosY;
			}
			nCurChar = cch;
			ShowCaret(hWnd);
			break;
		}
		SetCaretPos(nCaretPosX, nCaretPosY * dwCharY);
		break;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_LEFT:   // LEFT ARROW 

			// The caret can move only to the beginning of 
			// the current line. 

			if (nCaretPosX > 0)
			{
				HideCaret(hWnd);

				// Retrieve the character to the left of 
				// the caret, calculate the character's 
				// width, then subtract the width from the 
				// current horizontal position of the caret 
				// to obtain the new position. 

				ch = pchInputBuf[--nCurChar];
				hdc = GetDC(hWnd);
				GetCharWidth32(hdc, ch, ch, &nCharWidth);
				ReleaseDC(hWnd, hdc);
				nCaretPosX = max(nCaretPosX - nCharWidth,
					0);
				ShowCaret(hWnd);
			}
			break;

		case VK_RIGHT:  // RIGHT ARROW 

			// Caret moves to the right or, when a carriage 
			// return is encountered, to the beginning of 
			// the next line. 

			if (nCurChar < cch)
			{
				HideCaret(hWnd);

				// Retrieve the character to the right of 
				// the caret. If it's a carriage return, 
				// position the caret at the beginning of 
				// the next line. 

				ch = pchInputBuf[nCurChar];
				if (ch == 0x0D)
				{
					nCaretPosX = 0;
					nCaretPosY++;
				}

				// If the character isn't a carriage 
				// return, check to see whether the SHIFT 
				// key is down. If it is, invert the text 
				// colors and output the character. 

				else
				{
					hdc = GetDC(hWnd);
					nVirtKey = GetKeyState(VK_SHIFT);
					if (nVirtKey & SHIFTED)
					{
						crPrevText = SetTextColor(hdc,
							RGB(255, 255, 255));
						crPrevBk = SetBkColor(hdc,
							RGB(0, 0, 0));
						TextOut(hdc, nCaretPosX,
							nCaretPosY * dwCharY,
							&ch, 1);
						SetTextColor(hdc, crPrevText);
						SetBkColor(hdc, crPrevBk);
					}

					// Get the width of the character and 
					// calculate the new horizontal 
					// position of the caret. 

					GetCharWidth32(hdc, ch, ch, &nCharWidth);
					ReleaseDC(hWnd, hdc);
					nCaretPosX = nCaretPosX + nCharWidth;
				}
				nCurChar++;
				ShowCaret(hWnd);
				break;
			}
			break;

		case VK_UP:     // UP ARROW 
		case VK_DOWN:   // DOWN ARROW 
			MessageBeep((UINT)-1);
			return 0;

		case VK_HOME:   // HOME 
			// Set the caret's position to the upper left 
			// corner of the client area. 
			nCaretPosX = nCaretPosY = 0;
			nCurChar = 0;
			break;

		case VK_END:    // END  

			// Move the caret to the end of the text. 

			for (i = 0; i < cch; i++)
			{
				// Count the carriage returns and save the 
				// index of the last one. 

				if (pchInputBuf[i] == 0x0D)
				{
					cCR++;
					nCRIndex = i + 1;
				}
			}
			nCaretPosY = cCR;

			// Copy all text between the last carriage 
			// return and the end of the keyboard input 
			// buffer to a temporary buffer. 

			for (i = nCRIndex, j = 0; i < cch; i++, j++)
				szBuf[j] = pchInputBuf[i];
			szBuf[j] = TEXT('\0');

			// Retrieve the text extent and use it 
			// to set the horizontal position of the 
			// caret. 

			hdc = GetDC(hWnd);
			//HRESULT StringCchLength(LPTSTR psz, size_t cchMax, size_t *pcch);
			//pcch = 0;
			//hResult = StringCchLength(szBuf, 128, pcch);
			//if (FAILED(hResult))
			//{
			//	// TODO: write error handler
			//}
			/*GetTextExtentPoint32(hdc, szBuf, *pcch,
				&sz);*/
			//nCaretPosX = sz.cx;
			ReleaseDC(hWnd, hdc);
			nCurChar = cch;
			break;

		default:
			break;
		}
		SetCaretPos(nCaretPosX, nCaretPosY * dwCharY);
		break;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);

		TextOut(hdc,
			5, 5,
			windowMessage, _tcslen(windowMessage));

		EndPaint(hWnd, &ps);

		if (cch == 0)       // nothing in input buffer 
			break;

		hdc = BeginPaint(hWnd, &ps);
		HideCaret(hWnd);

		// Set the clipping rectangle, and then draw the text 
		// into it. 

		SetRect(&rc, 150, 150, dwLineLen, dwClientY);
		DrawText(hdc, pchInputBuf, -1, &rc, DT_LEFT);

		ShowCaret(hWnd);
		EndPaint(hWnd, &ps);
		break;

		// Process other messages. 

	case WM_DESTROY:
		PostQuitMessage(0);

		// Free the input buffer. 

		GlobalFree((HGLOBAL)pchInputBuf);
		UnregisterHotKey(hWnd, 0xAAAA);
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
		500, 500,
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
	BOOL bRet;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	//while ((bRet = GetMessage(&msg, (HWND)NULL, 0, 0)) != 0)
	//{
	//	if (bRet == -1)
	//	{
	//		// handle the error and possibly exit
	//	}
	//	else
	//	{
	//		/*if (TranslateAccelerator(hWnd, haccl, &msg) == 0)
	//		{*/
	//			TranslateMessage(&msg);
	//			DispatchMessage(&msg);
	//		//}
	//	}
	//}

	return (int)msg.wParam;
}
