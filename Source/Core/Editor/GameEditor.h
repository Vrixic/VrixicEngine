/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Core/Events/ApplicationEvents.h>
#include <Runtime/Engine/GameEngine.h>

#include <vector>

class VulkanDescriptorPool;
class VulkanDescriptorSetsLayout;
class VulkanFrameBuffer;
class VulkanRenderLayout;
class VulkanRenderPass;

/**
* A Game editor, consists of tools for use in a game engine, attaches itself to the engine 
*/
class VRIXIC_API VGameEditor
{
public:
	VGameEditor();

	~VGameEditor();

	/**
	* Initializes the game editor 
	*/
	void Init(TSharedPtr<VGameEngine> inGameEngine);

	/**
	* Updates the game editor
	*/
	void Tick();

	/**
	* Closes the game editor 
	*/
	void Shutdown();

	// --------------------------------------------------
	// Events
	// --------------------------------------------------

	/*
	* Called when a window event is fired
	* 
	* @param inWindowEvent the event to be handled that was fired 
	*/
	void OnEvent(WindowEvent& inWindowEvent);

	/**
	* Window resize event that is fired when the window resizes
	* 
	* @param inWindowResizeEvent the event that contains information about the resize 
	* @returns bool if the event was handled, if true, it will not get dropped, if false, it will go down the hierarchy until it is handled 
	*/
	bool OnWindowResized(WindowResizeEvent& inWindowResizeEvent);

private:
	TSharedPtr<VGameEngine> GameEngine;

	/** Create Imgui specific render pass which enables imgui to draw into our command buffer */
	VulkanRenderLayout* ImguiRenderLayout;
	VulkanRenderPass* ImguiRenderPassHandle;

	/** Since the renderpass is different for imgui, also create unqiue frame buffers for it as well */
	std::vector<VulkanFrameBuffer*> ImguiFrameBuffers;

	/** Descriptor layout and pool for imgui */
	VulkanDescriptorSetsLayout* ImguiDescriptorSetsLayout;
	VulkanDescriptorPool* ImguiDescriptorPool;
};
