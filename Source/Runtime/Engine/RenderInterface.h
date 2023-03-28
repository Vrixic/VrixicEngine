#pragma once
#include "GameWorld.h"
#include <Misc/Defines/GenericDefines.h>

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
class RenderInterface
{
public:
	/**
	* Initializes the renderer / render interface 
	*/
	virtual bool Init(const RendererInitializerList& inRenderInitializerList) = 0;

	/**
	* Begins a new render frame 
	*/
	virtual void BeginRenderFrame() = 0;

	/**
	* Render objects to the new render frame inside the game world 
	*/
	virtual void Render(GameWorld* inGameWorld) = 0;

	/**
	* End the render frame | MUST BE CALLED IF BEGIN RENDER FRAME WAS CALLED BEFORE CALLING ANOTHER BEGIN
	*/
	virtual void EndRenderFrame() = 0;

	/**
	* Begins a new render frame for editor GUI
	*/
	virtual void BeginEditorGuiRenderFrame() = 0;

	/**
	* End the render frame for editor GUI | MUST BE CALLED IF BEGIN RENDER FRAME WAS CALLED BEFORE CALLING ANOTHER BEGIN
	*/
	virtual void EndEditorGuiRenderFrame() = 0;

	/**
	* Called when the window resizes 
	*/
	virtual void OnRenderViewportResized(RenderViewportSize& inNewViewportSize) = 0;

	/**
	* Called when renderer is to be shutdowned
	*/
	virtual void Shutdown() = 0;
};
