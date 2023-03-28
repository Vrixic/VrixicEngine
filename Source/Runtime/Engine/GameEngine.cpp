#include "GameEngine.h"
#include <Runtime/Graphics/Vulkan/VulkanRenderer.h>
#include <Runtime/Memory/Core/MemoryManager.h>
#include <Core/Application.h>
#include <External/imgui/Includes/imgui.h>

#define VULKAN

VGameEngine::VGameEngine()
	: World(nullptr)
{
#ifdef VULKAN
	Renderer = TPointer<RenderInterface>((RenderInterface**)MemoryManager::Get().MallocConstructAligned<VulkanRenderer>(sizeof(VulkanRenderer), 8));
#endif // VULKAN

}

VGameEngine::~VGameEngine()
{
	Shutdown();
}

void VGameEngine::Init()
{
	// Initialize the renderer 
	RendererInitializerList RendererInitList = { 0 };

	RendererInitList.ViewportSize.Width = Application::ApplicationPtr->WindowPtr->GetWidth();
	RendererInitList.ViewportSize.Height = Application::ApplicationPtr->WindowPtr->GetHeight();
	RendererInitList.NativeWindowHandle = Application::ApplicationPtr->WindowPtr->GetNativeWindowHandle();
	RendererInitList.NativeWindowInstanceHandle = Application::ApplicationPtr->WindowPtr->GetNativeWindowInstanceHandle();

	Renderer.Get()->Init(RendererInitList);
}

void VGameEngine::Tick()
{
	// Start a new render frame 
	Renderer.Get()->BeginRenderFrame();

	// Render the world first 
	Renderer.Get()->Render(World);

	// Next start rendering the editor GUI
	Renderer.Get()->BeginEditorGuiRenderFrame();

	// Draw the game editor GUI/tools
	DrawEditorTools();

	// Stop the editor gui frame 
	Renderer.Get()->EndEditorGuiRenderFrame();

	// End of the current render frame and submit the frame 
	Renderer.Get()->EndRenderFrame();
}

void VGameEngine::DrawEditorTools()
{
	ImGui::ShowDemoWindow();
}

void VGameEngine::Shutdown()
{
	if (Renderer.IsValid())
	{
		Renderer.Get()->Shutdown();
		MemoryManager::Get().Free((void**)Renderer.GetRaw());
		Renderer.Free();
	}
}
