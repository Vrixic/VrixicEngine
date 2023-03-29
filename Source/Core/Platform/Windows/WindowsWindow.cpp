/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#include "WindowsWindow.h"

#include <Misc/Assert.h>
#include <Core/Events/ApplicationEvents.h>
#include <Core/Events/KeyEvent.h>
#include <Core/Events/MouseEvents.h>
#include <Misc/Logging/Log.h>
#include <Misc/Profiling/Profiler.h>

#define IMGUI
#ifdef IMGUI
#include <External/imgui/Includes/imgui_impl_win32.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#define LRESULT_IGNORE_VAL 0xfffffff0
#endif

#include <iostream>
#include <chrono>

#define MAKE_POINTS(lParam) const POINTS Point = MAKEPOINTS(lParam)

/**
* Makes it easy to make a mouse down event 
*/
#define WINDOW_PROC_MOUSE_DOWN_EVENT(mouseCode, lParam)			\
MAKE_POINTS(lParam);											\
MouseButtonPressedEvent Event(mouseCode, Point.x, Point.y);		\
OnInputEvent(Event);											

/**
* Makes it easy to make a mouse up event 
*/
#define WINDOW_PROC_MOUSE_UP_EVENT(mouseCode, lParam)			\
MAKE_POINTS(lParam);											\
MouseButtonReleasedEvent Event(mouseCode, Point.x, Point.y);	\
OnInputEvent(Event);	

// ---------------------------------------------------------------------
// -						Window class stuff						   -
// ---------------------------------------------------------------------

WindowsWindow::WindowClass::WindowClass()
	: WindowInstanceHandle(GetModuleHandle(nullptr)) { }

WindowsWindow::WindowClass::~WindowClass()
{
	Shutdown();
}

void WindowsWindow::WindowClass::Init(const std::wstring& inWindowClassName)
{
	WindowClassName = inWindowClassName;

	WNDCLASSEX WinClass = { 0 };
	WinClass.cbSize = sizeof(WNDCLASSEX);
	WinClass.style = CS_HREDRAW | CS_VREDRAW;
	WinClass.lpfnWndProc = HandleWindowsMessageSetup;
	WinClass.cbClsExtra = 0;
	WinClass.cbWndExtra = 0;
	WinClass.hInstance = WindowInstanceHandle;
	WinClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WinClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WinClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	WinClass.lpszMenuName = NULL;
	WinClass.lpszClassName = WindowClassName.c_str();
	WinClass.hIconSm = LoadIcon(NULL, IDI_WINLOGO);

	if (!RegisterClassEx(&WinClass))
	{
		VE_ASSERT(false, "Could not register window class!");
	}
}

void WindowsWindow::WindowClass::Shutdown()
{
	if (GetInstance() != nullptr)
	{
		UnregisterClass(WindowClassName.c_str(), GetInstance());
	}
}

const std::wstring& WindowsWindow::WindowClass::GetName()
{
	return WindowClassName;
}

HINSTANCE WindowsWindow::WindowClass::GetInstance()
{
	return WindowInstanceHandle;
}

// ---------------------------------------------------------------------
// -					 Windows Window Stuff						   -
// ---------------------------------------------------------------------

WindowsWindow::WindowsWindow(const FWindowConfig& inWindowConfig)
{
	VE_PROFILE_FUNCTION();

	Init(inWindowConfig);
}

WindowsWindow::~WindowsWindow()
{
	VE_PROFILE_FUNCTION();

	WindowClass::Get().Shutdown();
	Shutdown();
}

void WindowsWindow::Init(const FWindowConfig& inWindowConfig)
{
	VE_PROFILE_FUNCTION();

	// Set up window data 
	WindowsData.Name = inWindowConfig.Name;
	WindowsData.Width = inWindowConfig.Width;
	WindowsData.Height = inWindowConfig.Height;

	// Create the window 
	VE_CORE_LOG_INFO("Creating a Windows Window {0} ({1}, {2})", WindowsData.Name, WindowsData.Width, WindowsData.Height);

	std::wstring WindowName(WindowsData.Name.begin(), WindowsData.Name.end());

	/*WindowInstanceHandle = GetModuleHandle(0);

	WNDCLASSEX WindowClass;
	WindowClass.cbSize = sizeof(WNDCLASSEX);
	WindowClass.style = CS_HREDRAW | CS_VREDRAW;
	WindowClass.lpfnWndProc = HandleWindowsMessageSetup;
	WindowClass.cbClsExtra = 0;
	WindowClass.cbWndExtra = 0;
	WindowClass.hInstance = WindowInstanceHandle;
	WindowClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WindowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WindowClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	WindowClass.lpszMenuName = NULL;
	std::wstring WindowName(WindowsData.Name.begin(), WindowsData.Name.end());
	WindowClass.lpszClassName = WindowName.c_str();
	WindowClass.hIconSm = LoadIcon(NULL, IDI_WINLOGO);

	if (!RegisterClassEx(&WindowClass))
	{
		VE_ASSERT(false, "Could not register window class!");
		fflush(stdout);
		exit(1);
	}*/

	WindowClass::Get().Init(WindowName);

	int ScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	int ScreenHeight = GetSystemMetrics(SM_CYSCREEN);
	int WindowX = ScreenWidth / 2 - WindowsData.Width / 2;
	int WindowY = ScreenHeight / 2 - WindowsData.Height / 2;

	DWORD DwExStyle;
	DWORD DwStyle;

	/* Make the style a popup and u have a border less wwindow*/

	DwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE; //WS_POPUP
	DwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN; //WS_POPUP

	RECT WindowRect;
	WindowRect.left = 0L;
	WindowRect.top = 0L;
	WindowRect.right = (long)WindowsData.Width;
	WindowRect.bottom = (long)WindowsData.Height;

	AdjustWindowRectEx(&WindowRect, DwStyle, FALSE, DwExStyle);

	/*
		Last param 'this' is a custom parameter that you can set which you can get back when WINDOWPROC handler is first called (first message == WM_NCCREATE)
			-- it is stored in a struct 'CREATESTRUCTA' as 'lpCreateParams' which is the first lParam passed into the WINDOWPROC handler
	*/
	WindowHandle = CreateWindow(
		WindowName.c_str(),
		WindowName.c_str(),
		DwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		WindowX,
		WindowY,
		WindowRect.right - WindowRect.left,
		WindowRect.bottom - WindowRect.top,
		NULL,
		NULL,
		WindowClass::Get().GetInstance(),
		this); 

	if (!WindowHandle)
	{
		VE_ASSERT(false, "Could not create window!\n");
		fflush(stdout);
	}

	/* IMGUI STUFF */

#ifdef IMGUI

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGui_ImplWin32_Init(WindowHandle);

#endif // IMGUI

	/* IMGUI STUFF */

	ShowWindow(WindowHandle, SW_SHOW);
	SetForegroundWindow(WindowHandle);
	SetFocus(WindowHandle);

	bIsMouseInWindow = false;
}

LRESULT WINAPI WindowsWindow::HandleWindowsMessageSetup(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// use create parameter passed in from CreateWindow() to store window class pointer
	if (uMsg == WM_NCCREATE) {
		// extract ptr to window class from creation data
		const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
		WindowsWindow* const PtrToCurrentWindow = static_cast<WindowsWindow*>(pCreate->lpCreateParams);

		// set WinAPI-managed user data to store ptr to window class
		/*
			We want to store some data which is this Window class on the WindowsAPI side -> GWLP_USERDATA
			@Params: The window we want to set the data on, the type of data, the data to be stored inside of the WinAPI side.
		*/
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(PtrToCurrentWindow));

		// set message proc to normal (non-setup) handler now that setup is finished
		/*
			Now that we made a connection between this window and WinAPI we want to set a different WindowPROC which is HandleMessageThunk instead of this HandleMessageSetup
			* So now all messages will be handled by HandleMessageThunk instead of HandleMessageSetup
		*/
		SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&WindowsWindow::WindowsMessageHandler));

		// forward message to window instance handler
		/* Now that all the connections are made we forward the message to the actual message handler */
		return PtrToCurrentWindow->HandleWindowsMessage(hWnd, uMsg, wParam, lParam);
	}

	// if we get a message before the WM_NCCREATE message, handle with default handler
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT WINAPI WindowsWindow::WindowsMessageHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// retreive ptr to window instance
	/* Gets the pointer to our windows class that is stored on the WINAPI side then we call the windows handle message function */
	WindowsWindow* const PtrToCurrentWindow = reinterpret_cast<WindowsWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	// forward message to window instance handler
	return PtrToCurrentWindow->HandleWindowsMessage(hWnd, uMsg, wParam, lParam);
}

LRESULT WindowsWindow::HandleWindowsMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
#ifdef IMGUI
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
	{
		return LRESULT_IGNORE_VAL;
	}
#endif

	switch (uMsg)
	{
		/* Dont want window to close before we dont clean up out memory usage -- */
	case WM_CLOSE:
	{
		PostQuitMessage(0);
		WindowCloseEvent Event;
		OnInputEvent(Event);

		return LRESULT_IGNORE_VAL; // dont let windows destory the window for us, rather we do it ourselves 
	}

	case WM_SIZE:
		if (wParam != SIZE_MINIMIZED)
		{
			if (((wParam == SIZE_MAXIMIZED) || (wParam == SIZE_RESTORED)))
			{
				uint32 NewWidth = LOWORD(lParam);
				uint32 NewHeight = HIWORD(lParam);
				
				WindowResizeEvent Event(NewWidth, NewHeight);
				OnInputEvent(Event);
			}
		}
		break;

		/************** KEYBOARD MESSAGES **************/

	case WM_KEYDOWN:
		// syskey commands need to be handled to track ALT key (VK_MENU) and F10, regular WM_KEYDOWN does not handle those events
	case WM_SYSKEYDOWN:
	{
		// 0x40000000 =  bit 30 -> checking if key before this one was already pressed -> holding key down - > check msdn docs for more information
		KeyPressedEvent Event(static_cast<uint32>(wParam), lParam & 0xFF);
		OnInputEvent(Event);
		break;
	}

	case WM_KEYUP:
	case WM_SYSKEYUP:
	{
		KeyReleasedEvent Event(static_cast<uint32>(wParam));
		OnInputEvent(Event);
		break;
	}

	/************ END KEYBOARD MESSAGES ************/

	/************** MOUSE MESSAGES **************/

	case WM_MOUSEMOVE:
	{
		const POINTS Point = MAKEPOINTS(lParam);
		// in client region -> log move, and log enter + capture mouse (if not previously in window)
		if (Point.x >= 0 && Point.x < WindowsData.Width && Point.y >= 0 && Point.y < WindowsData.Height)
		{
			MouseMovedEvent Event(Point.x, Point.y);
			OnInputEvent(Event);
			if (!bIsMouseInWindow)
			{
				SetCapture(WindowHandle); // window function that tells WinAPI to log mouse events even if the cursor goes outside of window
				bIsMouseInWindow = true;
			}
		}
		// not in client -> log move / maintain capture if button down
		// if mouse move happens outside of the client region
		else
		{
			if (wParam & (MK_LBUTTON | MK_RBUTTON)) // check to see if the left button or the right button are currently down (wParam contains a bunch on info on states of buttons)
			{
				MouseMovedEvent Event(Point.x, Point.y);
				OnInputEvent(Event);
			}
			// button up -> release capture / log event for leaving
			else
			{
				ReleaseCapture();
				bIsMouseInWindow = false;
			}
		}
		break;
	}

	// All mouse down and up message sare repetive except this way its less branches and we get to configure the mouse code ourselves
	case WM_LBUTTONDOWN:
	{
		WINDOW_PROC_MOUSE_DOWN_EVENT(0, lParam);
		/*const POINTS Point = MAKEPOINTS(lParam);
		MouseButtonPressedEvent Event(0, Point.x, Point.y);
		OnInputEvent(Event);*/
		break;
	}
	case WM_RBUTTONDOWN:
	{
		WINDOW_PROC_MOUSE_DOWN_EVENT(1, lParam);
		/*const POINTS Point = MAKEPOINTS(lParam);
		MouseButtonPressedEvent Event(1, Point.x, Point.y);
		OnInputEvent(Event);*/
		break;
	}
	case WM_MBUTTONDOWN:
	{
		WINDOW_PROC_MOUSE_DOWN_EVENT(2, lParam);
		/*const POINTS Point = MAKEPOINTS(lParam);
		MouseButtonPressedEvent Event(2, Point.x, Point.y);
		OnInputEvent(Event);*/
		break;
	}
	case WM_XBUTTONDOWN:
	{
		WINDOW_PROC_MOUSE_DOWN_EVENT(((GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? 3 : 4), lParam);
		/*const POINTS Point = MAKEPOINTS(lParam);
		MouseButtonPressedEvent Event(((GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? 3 : 4), Point.x, Point.y);
		OnInputEvent(Event);*/
		break;
	}
	case WM_LBUTTONUP:
	{
		WINDOW_PROC_MOUSE_UP_EVENT(0, lParam);
		break;
	}
	case WM_RBUTTONUP:
	{
		WINDOW_PROC_MOUSE_UP_EVENT(1, lParam);
		break;
	}
	case WM_MBUTTONUP:
	{
		WINDOW_PROC_MOUSE_UP_EVENT(2, lParam);
		break;
	}
	case WM_XBUTTONUP:
	{
		WINDOW_PROC_MOUSE_UP_EVENT(((GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? 3 : 4), lParam);
		/*const POINTS Point = MAKEPOINTS(lParam);
		MouseButtonReleasedEvent Event(static_cast<uint32>(wParam), Point.x, Point.y);
		OnInputEvent(Event);*/
		break;
	}

	case WM_MOUSEWHEEL:
	{
		//const POINTS Point = MAKEPOINTS(lParam);
		const int VerticalDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		MouseScrolledEvent Event(0, VerticalDelta);
		OnInputEvent(Event);
		break;
	}

	/************ END MOUSE MESSAGES ************/
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void WindowsWindow::OnInputEvent(WindowEvent& inEvent)
{
	if (WindowsData.EventCallback != nullptr)
	{
		WindowsData.EventCallback(inEvent);
	}
}

void WindowsWindow::OnUpdate()
{
	VE_PROFILE_FUNCTION();

	MSG Message;

	if (PeekMessage(&Message, WindowHandle, 0, 0, PM_NOREMOVE))
	{
		GetMessage(&Message, NULL, 0, 0);
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}

	if (Message.message == WM_QUIT)
	{
		VE_CORE_LOG_INFO("Quit Message Posted");
	}
}

void WindowsWindow::Shutdown()
{
	VE_PROFILE_FUNCTION();

	DestroyWindow(WindowHandle);
}
