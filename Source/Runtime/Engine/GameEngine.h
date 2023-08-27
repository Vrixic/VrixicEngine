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

#include <External/enkiTS/Includes/TaskScheduler.h>

#include<chrono>

struct RunPinnedTaskLoopTask : enki::IPinnedTask {

    void Execute() override {
        while (TaskScheduler->GetIsRunning() && bShouldExecute) {
            TaskScheduler->WaitForNewPinnedTasks(); // this thread will 'sleep' until there are new pinned tasks
            TaskScheduler->RunPinnedTasks();
        }
    }

    enki::TaskScheduler* TaskScheduler;
    bool                  bShouldExecute = true;
}; 

struct AsynchronousLoadTask : enki::IPinnedTask
{
    void Execute() override;

    class AsynchronousLoader* Loader;
    enki::TaskScheduler* TaskScheduler;
    bool                        bShouldExecute = true;
};

/**
* The game engine, consists of all the modules and objects needed to run a game
*/
class VGameEngine
{
public:
    VGameEngine();

    ~VGameEngine();

    static VGameEngine* Get()
    {
        return GameEnginePtr;
    }

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

    uint64 GetFrameRate() const
    {
        return FrameRate;
    }

    float GetTickTime() const
    {
        return TickTime;
    }

    float GetRenderTime() const
    {
        return RenderTime;
    }

    enki::TaskScheduler& GetTaskScheduler()
    {
        return TaskScheduler;
    }

    AsynchronousLoader& GetAsyncLoader()
    {
        return *AsyncLoader;
    }

private:
    /** The application pointer */
    static VGameEngine* GameEnginePtr;

    bool bIsEngineActive;

    /** The Current world the engine is updating and rendering  */
    GameWorld* World;

    std::chrono::time_point<std::chrono::high_resolution_clock> LastTimestamp;

    uint64 FrameCounter;

    // Frame Rate in Millisecond 
    uint64 FrameRate;

    // Update Time
    float TickTime;

    // Render Time 
    float RenderTime;

    // TaskBased Multi-Threaded Rendering 
    enki::TaskScheduler TaskScheduler;

    RunPinnedTaskLoopTask RunPinnedTask = { };
    AsynchronousLoadTask AsyncLoadTask = { };
    class AsynchronousLoader* AsyncLoader;
};
