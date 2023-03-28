#pragma once
#include <Runtime/Engine/GameEngine.h>
#include <Core/Events/ApplicationEvents.h>

/**
* A Game editor, consists of tools for use in a game engine, attaches itself to the engine 
*/
class VGameEditor
{
private:
	TSharedPtr<VGameEngine> GameEngine;

public:
	VGameEditor();

	~VGameEditor();

	/**
	* Initializes the game editor 
	*/
	void Init(TSharedPtr<VGameEngine> inGameEngine);

	/**
	* Updates the game editor
	*/
	void Tick();

	/**
	* Closes the game editor 
	*/
	void Shutdown();

	// --------------------------------------------------
	// Events
	// --------------------------------------------------

	/*
	* Called when a window event is fired
	* 
	* @param inWindowEvent the event to be handled that was fired 
	*/
	void OnEvent(WindowEvent& inWindowEvent);

	/**
	* Window resize event that is fired when the window resizes
	* 
	* @param inWindowResizeEvent the event that contains information about the resize 
	* @returns bool if the event was handled, if true, it will not get dropped, if false, it will go down the hierarchy until it is handled 
	*/
	bool OnWindowResized(WindowResizeEvent& inWindowResizeEvent);

};
