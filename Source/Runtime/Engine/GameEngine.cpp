/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#include "GameEngine.h"
#include <Runtime/Graphics/Renderer.h>

VGameEngine::VGameEngine()
    : World(nullptr)
{
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
    Renderer::Get().Render();
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
