#include "GLFWWindowsWindow.h"
#include <Misc/Defines/StringDefines.h>
#include <Misc/Profiling/Profiler.h>
#include <Misc/Logging/Log.h>
#include <Misc/Assert.h>
#include <Core/Events/ApplicationEvents.h>
#include <Core/Events/KeyEvent.h>
#include <Core/Events/MouseEvents.h>

static uint8_t GLFWWindowCount = 0;

static void GLFWErrorCallback(int error, const char* description)
{
	VE_CORE_LOG_ERROR(VE_TEXT("GLFW Error ({0}): {1}"), error, description);
}

GLFWWindowsWindow::GLFWWindowsWindow(const FWindowConfig& inWindowConfig)
{
	VE_PROFILE_FUNCTION();

	Init(inWindowConfig);
}

GLFWWindowsWindow::~GLFWWindowsWindow()
{
	VE_PROFILE_FUNCTION();

	Shutdown();
}

void GLFWWindowsWindow::OnUpdate()
{
	VE_PROFILE_FUNCTION();

	glfwPollEvents();
}

void GLFWWindowsWindow::Init(const FWindowConfig& inWindowConfig)
{
	VE_PROFILE_FUNCTION();

	WindowsData.Name = inWindowConfig.Name;
	WindowsData.Width = inWindowConfig.Width;
	WindowsData.Height = inWindowConfig.Height;

	VE_CORE_LOG_INFO(VE_TEXT("Creating window {0} ({1}, {2})"), inWindowConfig.Name, inWindowConfig.Width, inWindowConfig.Height);

	if (GLFWWindowCount == 0)
	{
		int Success = glfwInit();
		VE_ASSERT(Success, "Could not initialize GLFW!");
		glfwSetErrorCallback(GLFWErrorCallback);
	}

	{ 
		// Create window with Vulkan context
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		WindowPtr = glfwCreateWindow((int)inWindowConfig.Width, (int)inWindowConfig.Height, inWindowConfig.Name.c_str(), nullptr, nullptr);
		++GLFWWindowCount;

		if (!glfwVulkanSupported())
		{
			VE_ASSERT(false, "GLFW: Vulkan Not Supported\n");
		}
	}

	glfwSetWindowUserPointer(WindowPtr, &WindowsData);
	// Set GLFW callbacks
	glfwSetWindowSizeCallback(WindowPtr, [](GLFWwindow* window, int width, int height)
		{
			FWindowData& data = *(FWindowData*)glfwGetWindowUserPointer(window);
			data.Width = width;
			data.Height = height;

			WindowResizeEvent event(width, height);
			data.EventCallback(event);
		});

	glfwSetWindowCloseCallback(WindowPtr, [](GLFWwindow* window)
		{
			FWindowData& data = *(FWindowData*)glfwGetWindowUserPointer(window);
			WindowCloseEvent event;
			data.EventCallback(event);
		});

	glfwSetKeyCallback(WindowPtr, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			FWindowData& data = *(FWindowData*)glfwGetWindowUserPointer(window);

			switch (action)
			{
			case GLFW_PRESS:
			{
				KeyPressedEvent event(key, 0);
				data.EventCallback(event);
				break;
			}
			case GLFW_RELEASE:
			{
				KeyReleasedEvent event(key);
				data.EventCallback(event);
				break;
			}
			case GLFW_REPEAT:
			{
				KeyPressedEvent event(key, true);
				data.EventCallback(event);
				break;
			}
			}
		});

	/*glfwSetCharCallback(WindowPtr, [](GLFWwindow* window, unsigned int keycode)
		{
			FWindowData& data = *(FWindowData*)glfwGetWindowUserPointer(window);

			KeyTypedEvent event(keycode);
			data.EventCallback(event);
		});*/

	glfwSetMouseButtonCallback(WindowPtr, [](GLFWwindow* window, int button, int action, int mods)
		{
			FWindowData& data = *(FWindowData*)glfwGetWindowUserPointer(window);

			switch (action)
			{
			case GLFW_PRESS:
			{
				MouseButtonPressedEvent event(button, data.MouseX, data.MouseY);
				data.EventCallback(event);
				break;
			}
			case GLFW_RELEASE:
			{
				MouseButtonReleasedEvent event(button, data.MouseX, data.MouseY);
				data.EventCallback(event);
				break;
			}
			}
		});

	glfwSetScrollCallback(WindowPtr, [](GLFWwindow* window, double xOffset, double yOffset)
		{
			FWindowData& data = *(FWindowData*)glfwGetWindowUserPointer(window);

			MouseScrolledEvent event((float)xOffset, (float)yOffset);
			data.EventCallback(event);
		});

	glfwSetCursorPosCallback(WindowPtr, [](GLFWwindow* window, double xPos, double yPos)
		{
			FWindowData& data = *(FWindowData*)glfwGetWindowUserPointer(window);

			MouseMovedEvent event((float)xPos, (float)yPos);
			data.EventCallback(event);

			data.MouseX = (uint16)xPos;
			data.MouseY = (uint16)yPos;
		});
}

void GLFWWindowsWindow::Shutdown()
{
	VE_PROFILE_FUNCTION();

	glfwDestroyWindow(WindowPtr);
	--GLFWWindowCount;

	if (GLFWWindowCount == 0)
	{
		glfwTerminate();
	}
}
