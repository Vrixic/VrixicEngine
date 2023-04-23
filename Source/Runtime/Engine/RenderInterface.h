/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include "GameWorld.h"
#include <Core/Misc/Interface.h> 
#include <Misc/Defines/GenericDefines.h>

#include <vector>

/**
* Graphics API that could be support by the engine 
*/
enum class EGraphicsInterface
{
	DirectX11,
	DirectX12,
	Vulkan
};

/** 
* All supported graphics interface, if a graphics interface is supported, it must include a renderer for itself, its resource specific management deriving from IResourceManager;
*/
//static std::vector<EGraphicsInterface> SupportedGraphicInterfaces = { EGraphicsInterface::Vulkan };

struct RenderViewportSize
{
	uint32 Width;
	uint32 Height;
};

struct RendererInitializerList
{
	// Window handle pointers for graphics api surface hook up
	void* NativeWindowHandle;
	void* NativeWindowInstanceHandle;

	// The render viewport size -> for now will be window size 
	RenderViewportSize ViewportSize;
};

/**
* Graphics API independent rendering interface 
*/
class IRenderSystem : Interface 
{
public:
	/**
	* Initializes the renderer / render interface 
	*/
	virtual bool Init(const RendererInitializerList& inRenderInitializerList) = 0;

	/**
	* Begins a new render frame
	* @warning do not call is twice in a row 
	*/
	virtual void BeginRenderFrame() = 0;

	/**
	* Begins listening to draw 
	* @param inCommandBufferIndex the index of the command buffer to list the command to (by default -1 = use internal next command buffer)
	* @remarks leave inCommandBufferIndex equal to -1 to use next buffer in line 
	*/
	virtual void BeginRecordingDrawCommands(int32 inCommandBufferIndex = -1) = 0;

	/**
	* Render objects to the new render frame inside the game world 
	*/
	virtual void Render(GameWorld* inGameWorld) = 0;

	/**
	* Ends listening to draw commands 
	* @param inCommandBufferIndex the index of the command buffer to end
	* @remarks no assumptions made to which command buffer to end, meaning expliticly tell renderer which one to end 
	*/
	virtual void EndRecordingDrawCommands(int32 inCommandBufferIndex) = 0;

	/**
	* End the render frame
	* @warning MUST BE CALLED IF BEGIN RENDER FRAME WAS CALLED BEFORE CALLING ANOTHER BEGIN
	*/
	virtual void EndRenderFrame() = 0;

	/**
	* Called when the window resizes 
	*/
	virtual void OnRenderViewportResized(RenderViewportSize& inNewViewportSize) = 0;

	/**
	* Called when renderer is to be shutdowned
	*/
	virtual void Shutdown() = 0;
};
