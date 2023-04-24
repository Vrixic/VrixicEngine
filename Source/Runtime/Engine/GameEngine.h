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
private:
	GameWorld* World; // The Current world the engine is updating and rendering 
	TPointer<IRenderInterface> RenderInterface; // renderer used to render things 

    /**
    * TESTING The render interface START
    */

    Surface* SurfacePtr;
    SwapChain* SwapChainMain;

    //
    std::vector<ICommandBuffer*> CommandBuffers;
    uint32 CurrentImageIndex;

    /* Synchronization */
    ISemaphore* PresentationCompleteSemaphore; // Swap chain image presentation
    ISemaphore* RenderCompleteSemaphore; // Command buffer submission and execution

    /* Depth and Stencil buffering */
    Texture* DepthStencilView;

    IRenderPass* RenderPass;

    // List of available frame buffers (same as number of swap chain images)
    std::vector<IFrameBuffer*> FrameBuffers;

    Shader* VertexShader;
    Shader* FragmentShader;

    // Create the pipeline layout, since we have no push constants nor descriptor sets, we just want an empty layout
    PipelineLayout* GraphicsPipelineLayout;

    // Create the graphics pipeline 
    IPipeline* GraphicsPipeline;

    Buffer* VertexBuffer;
    Buffer* IndexBuffer;

    /**
    * TESTING The render interface END
    */

	 
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
	TPointer<IRenderInterface>& GetRenderInterface() const { return (TPointer<IRenderInterface>&)RenderInterface; }
};
