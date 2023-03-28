#pragma once
#include "WindowEvent.h"

#include <sstream>

/**
* An event for when user moves the mouse, stores a 2D position of the mouse as well
*/
class VRIXIC_API MouseMovedEvent : public WindowEvent
{
private:
	uint16 MouseX, MouseY;

public:
	MouseMovedEvent(uint16 inMouseX, uint16 inMouseY)
		: MouseX(inMouseX), MouseY(inMouseY) { }

	inline uint16 GetMouseX() const { return MouseX; };
	inline uint16 GetMouseY() const { return MouseY; };

	std::string ToString() const override
	{
		std::stringstream SS;
		SS << "[MouseMovedEvent]: " << MouseX << ", " << MouseY;
		return SS.str();
	}

	WINDOW_EVENT_CLASS_TYPE(EWindowEventType::MouseMoved)
	WINDOW_EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
};

/**
* Mouse scrolled event for when user scrolls the mouse, keeps both vertical and horizontal scroll
* @Note: OffsetX will always be 0, need to add support for Horizontal Scroll
*/
class VRIXIC_API MouseScrolledEvent : public WindowEvent
{
private:
	float OffsetX, OffsetY;

public:
	MouseScrolledEvent(float inOffsetX, float inOffsetY)
		: OffsetX(inOffsetX), OffsetY(inOffsetY) { }

	inline float GetOffsetX() const { VE_CORE_LOG_WARN("No support for horizontal mouse scroll yet... GetOffsetX() will always return 0!"); return OffsetX; };
	inline float GetOffsetY() const { return OffsetY; };

	std::string ToString() const override
	{
		std::stringstream SS;
		SS << "[MouseScrolledEvent]: " << OffsetX << ", " << OffsetY;
		return SS.str();
	}

	WINDOW_EVENT_CLASS_TYPE(EWindowEventType::MouseScrolled)
	WINDOW_EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
};

/**
* Mouse button event, base class for mosue button press and release events, cannot be user created 
*/
class VRIXIC_API MouseButtonEvent : public WindowEvent
{
protected:
	uint32 Button;
	uint16 MouseX, MouseY;

	MouseButtonEvent(uint32 inButton, uint16 inMouseX, uint16 inMouseY)
		: Button(inButton), MouseX(inMouseX), MouseY(inMouseY) { }

public:
	inline uint32 GetMouseButton() const { return Button; }

	inline uint16 GetMouseX() const { return MouseX; };
	inline uint16 GetMouseY() const { return MouseY; };

	WINDOW_EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
};

/**
* An event for when the user press a mouse button, stores the button id
*/
class VRIXIC_API MouseButtonPressedEvent : public MouseButtonEvent
{
public:
	MouseButtonPressedEvent(uint32 inButton, uint16 inMouseX, uint16 inMouseY)
		: MouseButtonEvent(inButton, MouseX, MouseY) { }

	std::string ToString() const override
	{
		std::stringstream SS;
		SS << "[MouseButtonPressedEvent]: " << Button;
		return SS.str();
	}

	WINDOW_EVENT_CLASS_TYPE(EWindowEventType::MouseButtonPressed)
};

/**
* An event for when the user releases a mouse button, stores the button id
*/
class VRIXIC_API MouseButtonReleasedEvent : public MouseButtonEvent
{
public:
	MouseButtonReleasedEvent(uint32 inButton, uint16 inMouseX, uint16 inMouseY)
		: MouseButtonEvent(inButton, MouseX, MouseY) { }

	std::string ToString() const override
	{
		std::stringstream SS;
		SS << "[MouseButtonReleasedEvent]: " << Button;
		return SS.str();
	}

	WINDOW_EVENT_CLASS_TYPE(EWindowEventType::MouseButtonReleased)
};
