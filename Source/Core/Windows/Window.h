#pragma once

#include <Core/Core.h> 
#include <Core/Events/WindowEvent.h>
#include <Core/Misc/Interface.h> 
#include <Misc/Defines/GenericDefines.h>

#include <functional>

/**
* A platform independent window configuration struct 
*/
struct VRIXIC_API FWindowConfig
{
	std::string Name;

	uint32 Width;
	uint32 Height;

	FWindowConfig(const char* inName = "Vrixic Engine", uint32 inWidth = 1280, uint32 inHeight = 720)
		: Name(inName), Width(inWidth), Height(inHeight) { }
};

/**
* A window interface 
*/
class VRIXIC_API IWindow : public Interface
{
public:
	using EventCallbackFunc = std::function<void(WindowEvent&)>;
	IWindow();

	virtual ~IWindow();

	/**
	* Creates a platform specific window
	* @param inWindowConfig - The window configuration 
	* @returns TUniquePtr<Window> - a unique pointer to the created window 
	*/
	static TUniquePtr<IWindow> Create(const FWindowConfig& inWindowConfig = FWindowConfig());

	/**
	* Called every frame to update the window 
	*/
	virtual void OnUpdate() = 0;

	// ---------------------------------------------------------------------
	// -						 Window Attributes						   -
	// ---------------------------------------------------------------------
	virtual void SetEventCallback(const EventCallbackFunc& inCallback) = 0;

public:
	/* @returns uint32 - the width of the window */
	virtual uint32 GetWidth() const = 0;

	/* @returns uint32 - the height of the window */
	virtual uint32 GetHeight() const = 0;

	/* @returns void* - native window handle */
	virtual void* GetNativeWindowHandle() = 0;

	/* @returns void* - native window instance handle */
	virtual void* GetNativeWindowInstanceHandle() const = 0;

	/* @returns void* - native window instance handle for glfw window, only valid if glfw was used to create the window */
	virtual void* GetGLFWNativeHandle() const = 0;;
};
