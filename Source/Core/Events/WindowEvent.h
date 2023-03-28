#pragma once
#include <Misc/Defines/GenericDefines.h>
#include <Core/Core.h>

#include <string>
#include <functional>

/**
* @NOTE: Event system was initially created by @TheCherno(https://www.youtube.com/@TheCherno) in his game engine series - Big Thanks to Him!
*/

// Events implemented in such as a way that will not block when fired initially, instead
// they get put into a buffer which is read every frame 

/**
* An enum for event type for all window events that can happen
*/
enum class EWindowEventType
{
	None = 0,
	WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
	KeyPressed, KeyReleased,
	MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
};

/**
* Event category that can be used t filter specific event if need be 
*/
enum EWindowEventCategory
{
	None = 0,
	EventCategoryApplication		= BIT_SHIFT_LEFT(0),
	EventCategoryInput				= BIT_SHIFT_LEFT(1),
	EventCategoryKeyboard			= BIT_SHIFT_LEFT(2),
	EventCategoryMouse				= BIT_SHIFT_LEFT(3),
	EventCategoryMouseButton		= BIT_SHIFT_LEFT(4)
};

#define WINDOW_EVENT_CLASS_TYPE(type) \
static EWindowEventType GetStaticType() { return type; }\
virtual EWindowEventType GetEventType() const override { return GetStaticType(); }\
virtual const char* GetName() const override { return #type; }

#define WINDOW_EVENT_CLASS_CATEGORY(category) virtual int32 GetCategoryFlags() const override { return category; }

/**
* A generic window event 
*/
class VRIXIC_API WindowEvent
{
public:
	bool bIsHandled = false; // indicates if the event was handled if so will not be passed down in the hierarcy anymore Ex: UI button click, if clicked, should not go to game world 

public:
	virtual ~WindowEvent() = default;

	/* @returns EWindowEventType - the type of this event */
	virtual EWindowEventType GetEventType() const = 0;

	/* @returns const char* - event name */
	virtual const char* GetName() const = 0;

	/* @returns int32 - all of the category flags for this event */
	virtual int32 GetCategoryFlags() const = 0;

	/* @returns std::string - string of the event */
	virtual std::string ToString() const { return GetName(); }

	/**
	* Checks to see if this event is in a specific category 
	* @param inWindowEventCategory - the category to check if the event is in
	* @returns bool - true if it is in the category specified in param, false otherwise
	*/
	inline bool IsInCategory(EWindowEventCategory inWindowEventCategory)
	{
		return GetCategoryFlags() & inWindowEventCategory;
	}
};

/**
* An event dispatcher that will dispatch an event if the function provided matches the event type 
*/
class WindowEventDispatcher
{
private:
	template<typename EventType>
	using EventFunction = std::function<bool(EventType&)> ;

	WindowEvent& Event;

public:
	WindowEventDispatcher(WindowEvent& inWindowEvent)
		: Event(inWindowEvent) { }

	/**
	* Dispatches an event if the event type of the event matched up if the function parameter
	* @param inFunc - the function/callback to call if the event was matched up
	* @returns bool - true, if the event was fired, false otherwise 
	*/
	template<typename T>
	bool Dispatch(EventFunction<T> inFunc)
	{
		if (Event.GetEventType() == T::GetStaticType())
		{
			Event.bIsHandled |= inFunc(static_cast<T&>(Event));
			return true;
		}

		return false;
	}
};
