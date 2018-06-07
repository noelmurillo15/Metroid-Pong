
#include <Windows.h> // Include the Windows API (Win32 Application Programming Interface)
//#include <tchar.h> // Include UNICODE support
//#include <sstream> // Include ostringstream support
//using std::wostringstream;

//#include <commdlg.h> // Include the Common Dialogs.
#include <process.h> // Include Multi-Threading functions.
#include "resource.h" // Include the resource script IDs.
#include "Game.h" // Include the Game Class.


Game pong; // The Game Instance (only global temporarily).


// Foward Declarations:
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void OnPaint(HWND hWnd);
void OnLButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
void OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);


// Entry point function for the game:
unsigned int WINAPI GameMain(void* lpvArgList);


// The Entry Point function
int WINAPI _tWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance, // Always NULL
	LPTSTR	lpCmdLine,
	int		iCmdShow)
{
	// Initialize the COM Library.
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);


	// Step 1: Initialize the Window Class.
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(wcex);
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hInstance = hInstance;
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WindowProc; // Function to send Windows Message to for processing.
	wcex.lpszMenuName = MAKEINTRESOURCE(IDR_REALTIME);
	wcex.lpszClassName = _T("REALTIMEWND"); // Name to identify this Window Class.
	RegisterClassEx(&wcex);


	// Step 2: Create the Main Window.
	HWND hWnd = CreateWindow(wcex.lpszClassName,
		_T("Real-Time Skeleton"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		1200, 800,
		HWND_DESKTOP,
		NULL,
		hInstance,
		NULL);
	if (!hWnd)
	{
		//DWORD err = GetLastError();
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to create main window!"),
			_T("Program Name"), MB_OK | MB_ICONERROR);
		return -1;
	}


	// Step 2.5: Create the Game thread:
	pong.hGameThread = (HANDLE)_beginthreadex(NULL, 0,
		GameMain, (void*)hWnd, 0, NULL);
#ifdef _DEBUG
	Sleep(1);  // Sleep after thread creation for debugging to work properly.
#endif


	// Step 3: Show the Main Window.
	ShowWindow(hWnd, iCmdShow);
	UpdateWindow(hWnd);


	// Step 4: Enter the Main Message Loop.
	MSG msg = {};
	while (GetMessage(&msg, NULL, 0, 0)) // Getting WM_QUIT makes GetMessage return FALSE.
	{
		TranslateMessage(&msg); // Generate WM_CHAR messages (case sensitive input).
		DispatchMessage(&msg); // Invoke the WindowProc for message processing.
	}


	// Destroy the Game Thread:
	CloseHandle(pong.hGameThread);


	// Uninitialize the COM Library.
	CoUninitialize();


	return msg.wParam;
}


// Entry point function for the game:
unsigned int WINAPI GameMain(void* lpvArgList)
{
	// Initialize the COM Library.
	CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);

	// Increase the accuracy/precision of the Windows timer:
	TIMECAPS tc;
	timeGetDevCaps(&tc, sizeof(TIMECAPS));
	timeBeginPeriod(tc.wPeriodMin);


	// Get the HWND value out of the ArgList parameter.
	HWND hWnd = (HWND)lpvArgList;

	pong.SetHwnd(hWnd);
	pong.Startup();

	while (pong.GamePlay)
	{
		pong.Input();
		pong.Simulate();
		pong.Render();

		pong.EndGame();

		// 1 Frame == 1 iteration of Input, Simulate, & Render
	}

	pong.Shutdown();


	// Restore the accuracy/precision of the Windows timer:
	timeEndPeriod(tc.wPeriodMin);

	// Uninitialize the COM Library.
	CoUninitialize();

	return 0;
}


// Step 5: Write the Window Procedure
LRESULT CALLBACK WindowProc(HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam)
{
	switch (uMsg)
	{
		// Handle Window Creation:
	case WM_CREATE:
		break;

		// Handle Close Button [X] Pressed:
	case WM_CLOSE:
		// Begin GameMain shutdown.
		pong.GamePlay = FALSE;

		// Wait until Game Thread terminates.
		WaitForSingleObject(pong.hGameThread, INFINITE);

		// Begin WinMain shutdown.
		DestroyWindow(hWnd);
		break;

		// Handle Window Destruction:
	case WM_DESTROY:
		PostQuitMessage(0); // Put the WM_QUIT message in the message queue.
		break;


		// Handle Window Rendering:
	case WM_PAINT: //(output)
		OnPaint(hWnd);
		break;

	case WM_ERASEBKGND:
		// do nothing
		break;

		// Handle Left Mouse Button Clicked:
	case WM_LBUTTONDOWN: //(input)
		OnLButtonDown(hWnd, wParam, lParam);
		break;

		// Handle keyboard input:
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
		GetKeyboardState(pong.keys);
		DefWindowProc(hWnd, uMsg, wParam, lParam);
		break;

		// Handle window activation:
	case WM_ACTIVATE:
		if (LOWORD(wParam) != WA_INACTIVE)
			GetKeyboardState(pong.keys);
		else
			ZeroMemory(pong.keys, 256);
		break;

		// Handle Menu Items:
	case WM_COMMAND:
		OnCommand(hWnd, wParam, lParam);
		break;

	default:
		// Allow Windows to perform default processing.
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}

void OnPaint(HWND hWnd){
	ValidateRect(hWnd, NULL); // VERY IMPORTANT - Clear the Invalid flag
}

void OnLButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	RECT rc;
	GetClientRect(hWnd, &rc);

	POINT ptMouse;
	ptMouse.x = LOWORD(lParam); // Bits 0-15 contain the X Axis position
	ptMouse.y = HIWORD(lParam); // Bits 16-31 contain the Y Axis position

	InvalidateRect(hWnd, NULL, TRUE);
}

void OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	int menuid = LOWORD(wParam);
	switch (menuid)
	{
	case IDM_FILE_NEW:
		break;

	case IDM_FILE_EXIT:
		PostMessage(hWnd, WM_CLOSE, 0, 0);
		break;
	}
}
