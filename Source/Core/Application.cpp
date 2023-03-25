#include "Application.h"

#include <Misc/Assert.h>
#include <Misc/Profiling/Profiler.h>
#include <Runtime/Memory/Core/MemoryManager.h>

#include <crtdbg.h>
#include <Windows.h>
#include <chrono>

Application* Application::ApplicationPtr = nullptr;

Application::Application()
{
	VE_ASSERT(ApplicationPtr == nullptr, "Application should not be created twice! Application already exists!");
	ApplicationPtr = this;

	// Logging and Memory Output
#if _DEBUG
	_CrtDumpMemoryLeaks();
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
#endif // _DEBUG

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
	WindowPtr = Window::Create();
	VE_ASSERT(WindowPtr != nullptr, "Could not create a window!");

	WindowPtr->SetEventCallback(VE_BIND_EVENT_FUNC(Application::OnEvent));

	bIsRunning = true;
}

Application::~Application()
{
	VE_PROFILE_FUNCTION();

	// Startup the Memory Manager
	MemoryManager::Get().Shutdown();
}

void Application::OnEvent(WindowEvent& inEvent)
{
	VE_PROFILE_FUNCTION();

	VE_CORE_LOG_DISPLAY("Application::OnEvent: {0}", inEvent.ToString());

	if (inEvent.GetEventType() == EWindowEventType::WindowClose)
	{
		bIsRunning = false;
	}
}

void Application::Run()
{
	VE_PROFILE_FUNCTION();

	while (bIsRunning)
	{
		VE_PROFILE_BEGIN_SESSION("Main Loop");

		WindowPtr->OnUpdate();
	}

	VE_PROFILE_END_SESSION();
}
