#include "Window.h"

#include <Misc/Assert.h>

#ifdef PLATFORM_WINDOWS
#include <Core/Platform/Windows/GLFWWindowsWindow.h>
//#include <Core/Platform/Windows/WindowsWindow.h>
#endif // 

IWindow::IWindow()
{
}

IWindow::~IWindow()
{
}

TUniquePtr<IWindow> IWindow::Create(const FWindowConfig& inWindowConfig)
{
#ifdef PLATFORM_WINDOWS
	return CreateUniquePointer<GLFWWindowsWindow>(inWindowConfig);
#else
	VE_STATIC_ASSERT(false, "Platform is not known!");
	return nullptr;
#endif
}
