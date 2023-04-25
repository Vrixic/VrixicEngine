/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include "WindowEvent.h"

#include <sstream>

/**
* Base key event that encapsulates a KeyCode
*/
class VRIXIC_API KeyEvent : public WindowEvent
{
public:
	inline uint32 GetKeyCode() const { return KeyCode; }

	WINDOW_EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)

protected:
	uint32 KeyCode;

	KeyEvent(uint32 inKeyCode)
		: KeyCode(inKeyCode) { }
};

/**
* Key press event that derives form KeyEvent
*/
class VRIXIC_API KeyPressedEvent : public KeyEvent
{
public: 
	KeyPressedEvent(uint32 inKeyCode, uint32 inRepeatCount)
		: KeyEvent(inKeyCode), RepeatCount(inRepeatCount) { }

	std::string ToString() const override
	{
		std::stringstream SS;
		SS << "[KeyPressedEvent]: " << KeyCode << "(" << RepeatCount << " repeats)";
		return SS.str();
	}

	WINDOW_EVENT_CLASS_TYPE(EWindowEventType::KeyPressed)

protected:
	/** count of how many times they key is repeated, holding key down */
	uint32 RepeatCount; 
};

/**
* Key release event that derives form KeyEvent
*/
class VRIXIC_API KeyReleasedEvent : public KeyEvent
{
public:
	KeyReleasedEvent(uint32 inKeyCode)
		: KeyEvent(inKeyCode) { }

	std::string ToString() const override
	{
		std::stringstream SS;
		SS << "[KeyReleasedEvent]: " << KeyCode;
		return SS.str();
	}

	WINDOW_EVENT_CLASS_TYPE(EWindowEventType::KeyReleased)
};
