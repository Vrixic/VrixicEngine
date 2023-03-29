/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

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
	TPointer<IRenderSystem> Renderer; // renderer used to render things 
	 
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
	TPointer<IRenderSystem>& GetRenderer() const { return (TPointer<IRenderSystem>&)Renderer; }
};
