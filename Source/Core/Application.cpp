/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#include "Application.h"

#include <Misc/Assert.h>
#include <Misc/Defines/StringDefines.h>
#include <Misc/Profiling/Profiler.h>
#include "KeyCodes.h"
#include <Runtime/Memory/Core/MemoryManager.h>

#include <Windows.h>
#include <chrono>

Application* Application::ApplicationPtr = nullptr;

Application::Application()
{
	VE_ASSERT(ApplicationPtr == nullptr, "Application should not be created twice! Application already exists!");
	ApplicationPtr = this;

#if _EDITOR
	/* Allocate a console */
	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	FILE* Stream;
	freopen_s(&Stream, "CONIN$", "r", stdin);
	freopen_s(&Stream, "CONOUT$", "w+", stdout);
	freopen_s(&Stream, "CONOUT$", "w+", stderr);

	// Init Logger
	Log::Init();
#endif

	// Startup the Memory Manager
	MemoryManager::Get().StartUp();
	MemoryManager::Get().Resize(1024);

	// Create the window 
	WindowPtr = IWindow::Create();
	VE_ASSERT(WindowPtr != nullptr, "Could not create a window!");

	WindowPtr->SetEventCallback(VE_BIND_EVENT_FUNC(Application::OnEvent));

	bIsRunning = true;

	// Create the game engine and initialize it 
	GameEngine = CreateSharedPointer<VGameEngine>();
	GameEngine->Init();

	// if in editor mode create the editor for the engine 
//#ifdef _EDITOR
//	GameEditor = CreateUniquePointer<VGameEditor>();
//	GameEditor->Init(GameEngine);
//#endif // _EDITOR

}

Application::~Application()
{
	VE_PROFILE_FUNCTION();

//#if _EDITOR
//	GameEditor->Shutdown();
//#endif

	// Shutdown Game Engine
	GameEngine->Shutdown();

	// Shutdown the Memory Manager
	MemoryManager::Get().Shutdown();

	FreeConsole();
}

void Application::OnEvent(WindowEvent& inEvent)
{
	VE_PROFILE_FUNCTION();

	//VE_CORE_LOG_DISPLAY(VE_TEXT("App::OnEvent: {0}"), inEvent.ToString());

	WindowEventDispatcher EventDispatcher(inEvent);
 	EventDispatcher.Dispatch<KeyPressedEvent>(VE_BIND_EVENT_FUNC(OnKeyDownEvent));

	if (inEvent.GetEventType() == EWindowEventType::WindowClose)
	{
		bIsRunning = false;
	}

	GameEngine->OnWindowEvent(inEvent);

//#if _EDITOR
//	GameEditor->OnEvent(inEvent);
//#endif
}

bool Application::OnKeyDownEvent(KeyPressedEvent& inKeyEvent)
{
	if (inKeyEvent.GetKeyCode() == Key::Escape)
	{
		bIsRunning = false;
		return true;
	}

	return false;
}

void Application::Run()
{
	VE_PROFILE_FUNCTION();

	while (bIsRunning)
	{
		VE_PROFILE_BEGIN_SESSION("Main Loop");

		//WindowPtr->OnUpdate();

//#if _EDITOR
//		GameEditor->Tick();
//#else	
//		GameEngine->Tick();
//#endif // _EDITOR

		GameEngine->Tick();
	}

	VE_PROFILE_END_SESSION();
}
