/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include "GameWorld.h"
#include <Runtime/Graphics/IRenderInterface.h>
#include <Core/Events/ApplicationEvents.h>
#include <Core/Events/KeyEvent.h>
#include <Core/Events/MouseEvents.h>

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

    /**
    * Called when a window event gets fired
    * 
    * @param inWindowEvent the window event that got fired 
    */
    void OnWindowEvent(WindowEvent& inWindowEvent);

    /**
    * Called when window gets resized
    * 
    * @param inWindowResizeEvent the window resize event 
    */
    bool OnWindowResized(WindowResizeEvent& inWindowResizeEvent);

    bool OnMouseButtonPressed(MouseButtonPressedEvent& inMouseEvent);
    bool OnMouseButtonReleased(MouseButtonReleasedEvent& inMouseEvent);
    bool OnMouseMoved(MouseMovedEvent& inMouseEvent);
    bool OnMouseScrolled(MouseScrolledEvent& inMouseEvent);

    bool OnKeyPressed(KeyPressedEvent& inKeyPressedEvent);
    bool OnKeyReleased(KeyReleasedEvent& inKeyReleasedEvent);

private:
    /** The Current world the engine is updating and rendering  */
    GameWorld* World;
};
