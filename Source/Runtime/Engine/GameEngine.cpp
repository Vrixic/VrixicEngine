/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#include "GameEngine.h"
#include <Runtime/Graphics/Renderer.h>

#include <Core/Application.h>
#include <Misc/Assert.h>

VGameEngine* VGameEngine::GameEnginePtr = nullptr;

VGameEngine::VGameEngine()
    : World(nullptr), RenderTime(0), FrameRate(0), TickTime(0)
{
    VE_ASSERT(GameEnginePtr == nullptr, "Game Engine should not be created twice! Game Engine already exists!");
    GameEnginePtr = this;
}

VGameEngine::~VGameEngine()
{
    Shutdown();
}

void VGameEngine::Init()
{
    // Initialize the renderer 
    FRendererConfig Config = {};
    Config.RenderInterfaceType = ERenderInterfaceType::Vulkan;
    Config.bEnableRenderDoc = false;

    Renderer::Get().Init(Config);
}

void VGameEngine::Tick()
{
    typedef std::chrono::high_resolution_clock Clock;
    auto Start = Clock::now();

    FrameCounter++;

    Application::Get()->GetWindow().OnUpdate();

    TickTime = std::chrono::duration<float, std::milli>(Clock::now() - Start).count();
    TickTime /= 1000.0f;

    auto RenderStart = Clock::now();

    Renderer::Get().Render();

    auto End = Clock::now();

    RenderTime = std::chrono::duration<float, std::milli>(End - RenderStart).count();
    RenderTime /= 1000.0f;

    float FpsTimer = (std::chrono::duration<float, std::milli>(End - LastTimestamp).count());
    if (FpsTimer > 1000.0f)
    {
        FrameRate = static_cast<uint64>((float)FrameCounter * (1000.0f / FpsTimer));
        FrameCounter = 0;
        LastTimestamp = End;
    }
}

void VGameEngine::Shutdown()
{
    Renderer::Get().Shutdown();
}

void VGameEngine::OnWindowEvent(WindowEvent& inWindowEvent)
{
    WindowEventDispatcher EventDispatcher(inWindowEvent);
    EventDispatcher.Dispatch<WindowResizeEvent>(VE_BIND_EVENT_FUNC(OnWindowResized));
    EventDispatcher.Dispatch<MouseMovedEvent>(VE_BIND_EVENT_FUNC(OnMouseMoved));
    EventDispatcher.Dispatch<MouseButtonPressedEvent>(VE_BIND_EVENT_FUNC(OnMouseButtonPressed));
    EventDispatcher.Dispatch<MouseButtonReleasedEvent>(VE_BIND_EVENT_FUNC(OnMouseButtonReleased));
    EventDispatcher.Dispatch<MouseScrolledEvent>(VE_BIND_EVENT_FUNC(OnMouseScrolled));
    EventDispatcher.Dispatch<KeyPressedEvent>(VE_BIND_EVENT_FUNC(OnKeyPressed));
    EventDispatcher.Dispatch<KeyReleasedEvent>(VE_BIND_EVENT_FUNC(OnKeyReleased));
}

bool VGameEngine::OnWindowResized(WindowResizeEvent& inWindowResizeEvent)
{
    FExtent2D Extent2D = { inWindowResizeEvent.GetWidth(), inWindowResizeEvent.GetHeight() };
    return Renderer::Get().OnRenderViewportResized(Extent2D);
}

bool VGameEngine::OnMouseButtonPressed(MouseButtonPressedEvent& inMouseEvent)
{
    return Renderer::Get().OnMouseButtonPressed(inMouseEvent);
}

bool VGameEngine::OnMouseButtonReleased(MouseButtonReleasedEvent& inMouseEvent)
{
    return Renderer::Get().OnMouseButtonReleased(inMouseEvent);
}

bool VGameEngine::OnMouseMoved(MouseMovedEvent& inMouseEvent)
{
    return Renderer::Get().OnMouseMoved(inMouseEvent);
}

bool VGameEngine::OnMouseScrolled(MouseScrolledEvent& inMouseEvent)
{
    return Renderer::Get().OnMouseScrolled(inMouseEvent);
}

bool VGameEngine::OnKeyPressed(KeyPressedEvent& inKeyPressedEvent)
{
    return Renderer::Get().OnKeyPressed(inKeyPressedEvent);
}

bool VGameEngine::OnKeyReleased(KeyReleasedEvent& inKeyReleasedEvent)
{
    return Renderer::Get().OnKeyReleased(inKeyReleasedEvent);
}
