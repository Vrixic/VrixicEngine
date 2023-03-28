#pragma once

#include <Core/Events/WindowEvent.h>
#include <Core/Windows/Window.h>

#include <Windows.h>
#include <queue>

/**
* @NOTE: The work around for windows input event system (WNDPROC) was created and used from @ChiliTomatoNoodle (https://www.youtube.com/user/ChiliTomatoNoodle) in his game engine series - Big Thanks to Him!
*/

/**
* Representation of a windows platform window 
*/
class VRIXIC_API WindowsWindow : public Window
{
private:
	/**
	* All data a windows window would need or have 
	*/
	struct VRIXIC_API FWindowData
	{
		std::string Name;
		uint32 Width, Height;

		EventCallbackFunc EventCallback;
	};
	FWindowData WindowsData;

	HWND WindowHandle; // The handle to the window itself 

	// Keeps track if the mouse if currently inside the rect of the window or outside 
	bool bIsMouseInWindow;
public:
	WindowsWindow(const FWindowConfig& inWindowConfig);

	virtual ~WindowsWindow();

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

private:
	/** Singleton class that manages registration and cleanup of window classes */
	class VRIXIC_API WindowClass
	{
	private:
		std::wstring WindowClassName; 
		HINSTANCE WindowInstanceHandle;

	public:
		/**
		* Returns the one and only Instance 
		*/
		static WindowClass& Get()
		{
			static WindowClass Instance; // Handle to the singleton
			return Instance;
		}

		/**
		* Initliazes the window class
		*/
		void Init(const std::wstring& inWindowClassName);

		/**
		* Close out the window instance and clean up 
		*/
		void Shutdown();

		const std::wstring& GetName();
		HINSTANCE GetInstance();

	private:
		WindowClass();
		~WindowClass();

		WindowClass(const WindowClass&) = delete;
		WindowClass& operator=(const WindowClass&) = delete;
	};

	/**
	* Initializes and creates the window with the configuration passed in
	* @param inWindowConfig - the configuration of the window to be created 
	*/
	virtual void Init(const FWindowConfig& inWindowConfig);

	virtual void Shutdown();

	/**
	* The callback that is registered to the window upon creation 
	*/
	static LRESULT CALLBACK HandleWindowsMessageSetup(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	/**
	* This functions job is just to invoke our WindowProc function as WINAPI(win32) cannot directly do it
	*/
	static LRESULT CALLBACK WindowsMessageHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	/**
	* Handles all the incomming messages from the window 
	*/
	LRESULT HandleWindowsMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	/**
	* Called when an event happens
	* @param inEvent - the event to be handled/pushed
	*/
	void OnInputEvent(WindowEvent& inEvent);
public:
	virtual uint32 GetWidth() const override { return WindowsData.Width; };

	virtual uint32 GetHeight() const override { return WindowsData.Height; };

	virtual void* GetNativeWindowHandle() const override { return static_cast<void*>(&(HWND)WindowHandle); };

	virtual void* GetNativeWindowInstanceHandle() const override { return WindowClass::Get().GetInstance(); };
};
