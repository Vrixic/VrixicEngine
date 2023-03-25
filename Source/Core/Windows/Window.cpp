#include "Window.h"

#include <Misc/Assert.h>

#ifdef PLATFORM_WINDOWS
#include <Core/Platform/Windows/WindowsWindow.h>
#endif // 


Window::Window()
{
}

Window::~Window()
{
}

TUniquePtr<Window> Window::Create(const FWindowConfig& inWindowConfig)
{
#ifdef PLATFORM_WINDOWS
	return CreateUniquePointer<WindowsWindow>(inWindowConfig);
#else
	VE_STATIC_ASSERT(false, "Platform is not known!");
	return nullptr;
#endif
}
