/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include "GameWorld.h"
#include <Runtime/Graphics/IRenderInterface.h>

/**
* The game engine, consists of all the modules and objects needed to run a game
*/
class VGameEngine
{
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
    * Closes the game engine
    */
    void Shutdown();

private:
    /** The Current world the engine is updating and rendering  */
    GameWorld* World;
};
