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
