#pragma once
#include "GameWorld.h"
#include "RenderInterface.h"

/**
* The game engine, consists of all the modules and objects needed to run a game
*/
class VGameEngine
{
private:
	GameWorld* World; // The Current world the engine is updating and rendering 
	TPointer<RenderInterface> Renderer; // renderer used to render things 
	 
public:
	VGameEngine();

	~VGameEngine();

	/**
	* Initializes the game engine
	*/
	void Init();

	/**
	* Updates the game engine
	*/
	void Tick();

	/**
	* Draws game editor tools
	*/
	void DrawEditorTools();

	/**
	* Closes the game engine
	*/
	void Shutdown();

public:
	TPointer<RenderInterface>& GetRenderer() const { return (TPointer<RenderInterface>&)Renderer; }
};
