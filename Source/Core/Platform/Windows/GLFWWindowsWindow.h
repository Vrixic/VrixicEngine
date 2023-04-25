/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Core/Windows/Window.h>

#include <External/glfw/Includes/GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32 
#include <External/glfw/Includes/GLFW/glfw3native.h>

/**
* Glfws implementation of a windows window 
*/
class GLFWWindowsWindow : public IWindow
{
public:
	GLFWWindowsWindow(const FWindowConfig& inWindowConfig);

	virtual ~GLFWWindowsWindow();

	/**
	* Called every frame to update the window
	*/
	virtual void OnUpdate() override;

	/**
	* Sets the event callback to use when the window pushes a message which is then Handled by HandleWindowsMessage()
	*/
	virtual void SetEventCallback(const EventCallbackFunc& inCallback) override
	{
		WindowsData.EventCallback = inCallback;
	}

public:
	virtual uint32 GetWidth() const override { return WindowsData.Width; };

	virtual uint32 GetHeight() const override { return WindowsData.Height; };

	virtual void* GetNativeWindowHandle() override 
	{ 
		WindowHandle = glfwGetWin32Window(WindowPtr);
		return static_cast<void*>(&(HWND)WindowHandle);
	};

	virtual void* GetNativeWindowInstanceHandle() const override { return GetModuleHandle(0); };

	virtual void* GetGLFWNativeHandle() const override { return WindowPtr; }

private:
	/**
	* Initializes and creates the window with the configuration passed in
	* @param inWindowConfig - the configuration of the window to be created
	*/
	virtual void Init(const FWindowConfig& inWindowConfig);

	virtual void Shutdown();

	/**
	* Called when an event happens
	* @param inEvent - the event to be handled/pushed
	*/
	/*void OnInputEvent(WindowEvent& inEvent);*/

private:
	/**
	* All data a windows window would need or have
	* Helper Structure for storing window data 
	*/
	struct VRIXIC_API HWindowData
	{
	public:
		std::string Name;
		uint32 Width, Height;

		uint16 MouseX, MouseY;

		EventCallbackFunc EventCallback;

	public:
		HWindowData()
			: Width(0), Height(0), MouseX(0), MouseY(0) { }
	};

	HWindowData WindowsData;

	GLFWwindow* WindowPtr;

	HWND WindowHandle;
};