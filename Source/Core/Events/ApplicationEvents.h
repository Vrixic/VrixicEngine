#pragma once
#include <Core/Core.h>
#include "WindowEvent.h"

#include <sstream>

/**
* An event for when the user resizes the window, contains the new width and height 
*/
class VRIXIC_API WindowResizeEvent : public WindowEvent
{
private:
	uint32 Width, Height;

public:
	WindowResizeEvent(uint32 inNewWidth, uint32 inNewHeight)
		: Width(inNewWidth), Height(inNewHeight) { }

	inline uint32 GetWidth() const { return Width; }
	inline uint32 GetHeight() const { return Height; }

	std::string ToString() const override
	{
		std::stringstream SS;
		SS << "[WindowResizeEvent]: " << Width << " " << Height;
		return SS.str();
	}

	WINDOW_EVENT_CLASS_TYPE(EWindowEventType::WindowResize)
	WINDOW_EVENT_CLASS_CATEGORY(EventCategoryApplication)
};

/**
* An event for when the window closes 
*/
class VRIXIC_API WindowCloseEvent : public WindowEvent
{
public:
	WindowCloseEvent() { }

	WINDOW_EVENT_CLASS_TYPE(EWindowEventType::WindowClose)
	WINDOW_EVENT_CLASS_CATEGORY(EventCategoryApplication)
};