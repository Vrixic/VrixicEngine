///**
//* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
//* See "LICENSE.txt" for license information.
//*/
//
//#include "GameEditor.h"
//#include <Core/Application.h>
//#include <Misc/Profiling/Profiler.h>
//#include <Runtime/Graphics/Vulkan/VulkanRenderer.h>
//
//#include <External/imgui/Includes/imgui.h>
//#include <External/imgui/Includes/imgui_impl_glfw.h>
//#include <External/imgui/Includes/imgui_impl_vulkan.h>
//
//#include <External/glfw/Includes/GLFW/glfw3.h>
//
//static void ImguiCheckVkResultFunc(VkResult err)
//{
//	VK_CHECK_RESULT(err, "[ImguiVulkanImpInit]: failed");
//}
//
//VGameEditor::VGameEditor()
//{
//	VE_PROFILE_FUNCTION();
//	GameEngine = nullptr;
//}
//
//VGameEditor::~VGameEditor()
//{
//	VE_PROFILE_FUNCTION();
//
//	if (VulkanRenderer::Get() != nullptr) // Can still have to release out side of objects 
//	{
//		Shutdown();
//	}
//}
//
//void VGameEditor::Init(TSharedPtr<VGameEngine> inGameEngine)
//{
//	VE_PROFILE_FUNCTION();
//
//	GameEngine = inGameEngine;
//
//	//// attach imgui to game editor
//#ifdef VULKAN_GLFW
//
//	// Create Imgui renderpass and frame buffers and descriptor pools
//	{
//		// Render Layout and Renderpass creation
//		{
//			VkRect2D RenderArea = { {0,0},
//				{ Application::Get()->GetWindow().GetWidth(), Application::Get()->GetWindow().GetHeight()} };
//			ImguiRenderLayout = new VulkanRenderLayout(VulkanRenderer::Get()->Device, 1, RenderArea);
//
//			// attachments 
//			{
//				std::vector<VkAttachmentDescription> Attachment;
//				Attachment.resize(1);
//				Attachment[0].format = VulkanRenderer::Get()->Surface->GetSurfaceFormat()->format;
//				Attachment[0].samples = VK_SAMPLE_COUNT_1_BIT;
//				Attachment[0].loadOp = 0 ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//				Attachment[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
//				Attachment[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//				Attachment[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//				Attachment[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//				Attachment[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
//
//				ImguiRenderLayout->SetAttachments(Attachment);
//
//				VkAttachmentReference ColorAttachment;
//				ColorAttachment.attachment = 0;
//				ColorAttachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//
//				ImguiRenderLayout->SetColorReference(ColorAttachment);
//			}
//
//			// Subpass dependency 
//			std::vector<VkSubpassDependency> SubpassDependency;
//			SubpassDependency.resize(1);
//			SubpassDependency[0].srcSubpass = VK_SUBPASS_EXTERNAL;
//			SubpassDependency[0].dstSubpass = 0;
//			SubpassDependency[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//			SubpassDependency[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//			SubpassDependency[0].srcAccessMask = 0;
//			SubpassDependency[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//
//			// Create renderpass from render layout and subpass dependency 
//			ImguiRenderPassHandle = new VulkanRenderPass(VulkanRenderer::Get()->Device, *ImguiRenderLayout, SubpassDependency);
//		}
//
//		VE_CORE_LOG_INFO("[GameEditor]: Successfully created imgui renderpass...");
//
//		// Frame Buffers creation
//		{
//			VulkanSwapChain const* Swapchain = VulkanRenderer::Get()->GetVulkanSwapchain();
//
//			VkImageView Attachment[1];
//			VkExtent2D Extent = { Application::Get()->GetWindow().GetWidth(), Application::Get()->GetWindow().GetHeight() };
//
//			ImguiFrameBuffers.resize(Swapchain->GetImageCount());
//			for (uint32_t i = 0; i < Swapchain->GetImageCount(); i++)
//			{
//				Attachment[0] = Swapchain->GetSwapchainBuffer(i)->View;
//				ImguiFrameBuffers[i] = new VulkanFrameBuffer(VulkanRenderer::Get()->Device, ImguiRenderPassHandle);
//				ImguiFrameBuffers[i]->Create(1, Attachment, &Extent);
//			}
//		}
//
//		VE_CORE_LOG_INFO("[GameEditor]: Successfully created imgui framebuffers...");
//
//		// Descriptor Sets layout and descriptor pool creation
//		{
//			std::vector<VkDescriptorPoolSize> pool_sizes =
//			{
//				{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
//				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
//				{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
//				{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
//				{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
//				{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
//				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
//				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
//				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
//				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
//				{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
//			};
//
//			ImguiDescriptorSetsLayout = new VulkanDescriptorSetsLayout(VulkanRenderer::Get()->Device);
//			ImguiDescriptorPool = new VulkanDescriptorPool(VulkanRenderer::Get()->Device, *ImguiDescriptorSetsLayout, 1000 * 11, pool_sizes);
//		}
//
//		VE_CORE_LOG_INFO("[GameEditor]: Successfully created imgui descriptor pool...");
//	}
//
//	// Setup Dear ImGui context
//	{
//		IMGUI_CHECKVERSION();
//		ImGui::CreateContext();
//		ImGuiIO& io = ImGui::GetIO(); (void)io;
//		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
//		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
//		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
//		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
//		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
//		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;
//
//		// Setup Dear ImGui style
//		ImGui::StyleColorsDark();
//		//ImGui::StyleColorsLight();
//
//		// Setup Platform/Renderer backends
//		ImGui_ImplGlfw_InitForVulkan((GLFWwindow*)Application::Get()->GetWindow().GetGLFWNativeHandle(), true);
//		ImGui_ImplVulkan_InitInfo ImguiVulkanInitInfo = { 0 };
//		ImguiVulkanInitInfo.Instance = *VulkanRenderer::Get()->GetVulkanInstance();
//		ImguiVulkanInitInfo.PhysicalDevice = *VulkanRenderer::Get()->GetVulkanDevice()->GetPhysicalDeviceHandle();
//		ImguiVulkanInitInfo.Device = *VulkanRenderer::Get()->GetVulkanDevice()->GetDeviceHandle();
//		ImguiVulkanInitInfo.QueueFamily = VulkanRenderer::Get()->GetVulkanDevice()->GetGraphicsQueue()->GetFamilyIndex();
//		ImguiVulkanInitInfo.Queue = VulkanRenderer::Get()->GetVulkanDevice()->GetGraphicsQueue()->GetQueueHandle();
//		ImguiVulkanInitInfo.PipelineCache = VulkanRenderer::Get()->PipelineCache;
//		ImguiVulkanInitInfo.DescriptorPool = *ImguiDescriptorPool->GetDescriptorPoolHandle();
//		ImguiVulkanInitInfo.Subpass = 0;
//		ImguiVulkanInitInfo.MinImageCount = 2;
//		ImguiVulkanInitInfo.ImageCount = VulkanRenderer::Get()->GetVulkanSwapchain()->GetImageCount();
//		ImguiVulkanInitInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
//		ImguiVulkanInitInfo.Allocator = nullptr;
//		ImguiVulkanInitInfo.CheckVkResultFn = ImguiCheckVkResultFunc;
//		ImGui_ImplVulkan_Init(&ImguiVulkanInitInfo, *ImguiRenderPassHandle->GetRenderPassHandle());
//	}
//
//	VE_CORE_LOG_INFO("[GameEditor]: Successfully initialized imgui for vulkan...");
//
//	// Upload Fonts
//	{
//		//// Load Fonts
//		// // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
//		// // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
//		// // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
//		// // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
//		// // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
//		// // - Read 'docs/FONTS.md' for more instructions and details.
//		// // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
//		// //io.Fonts->AddFontDefault();
//		// //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
//		// //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
//		// //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
//		// //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
//		// //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
//		// //IM_ASSERT(font != NULL);
//		// 
//		// Use any command queue
//		VkCommandPool command_pool = VulkanRenderer::Get()->GetCommandPool()->GetCommandPoolHandle();
//		VkCommandBuffer command_buffer = *VulkanRenderer::Get()->GetCommandPool()->GetCommandBuffer(0)->GetCommandBufferHandle();
//
//		VK_CHECK_RESULT(vkResetCommandPool(*VulkanRenderer::Get()->GetVulkanDevice()->GetDeviceHandle(), command_pool, 0), "[]");
//		VkCommandBufferBeginInfo begin_info = {};
//		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//		begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
//		VK_CHECK_RESULT(vkBeginCommandBuffer(command_buffer, &begin_info), "");
//
//		ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
//
//		VkSubmitInfo end_info = {};
//		end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//		end_info.commandBufferCount = 1;
//		end_info.pCommandBuffers = &command_buffer;
//		VK_CHECK_RESULT(vkEndCommandBuffer(command_buffer), "");
//		VK_CHECK_RESULT(vkQueueSubmit(VulkanRenderer::Get()->GetVulkanDevice()->GetGraphicsQueue()->GetQueueHandle(), 1, &end_info, VK_NULL_HANDLE), "");
//
//		VulkanRenderer::Get()->GetVulkanDevice()->WaitUntilIdle();
//		ImGui_ImplVulkan_DestroyFontUploadObjects();
//
//		VE_CORE_LOG_INFO("[GameEditor]: Successfully uploaded default imgui font...");
//	}
//
//#endif // GLFW_VULKAN_WINDOWS
//}
//
//void VGameEditor::Tick()
//{
//	VE_PROFILE_FUNCTION();
//	bool show_demo_window = true;
//
//	//GameEngine->Tick();
//
//	// Start a new render frame 
//	VulkanRenderer::Get()->BeginRenderFrame();
//	VulkanRenderer::Get()->BeginCommandBuffer();
//	VulkanRenderer::Get()->BeginRenderPass(VulkanRenderer::Get()->RenderPass);
//
//	// Render the world first 
//	VulkanRenderer::Get()->Render(nullptr);
//
//	VulkanRenderer::Get()->EndRenderPass();
//
//	// Start the Dear ImGui frame
//	ImGui_ImplVulkan_NewFrame();
//	ImGui_ImplGlfw_NewFrame();
//	ImGui::NewFrame();
//
//	ImGui::ShowDemoWindow(&show_demo_window);
//
//	ImGuiIO& io = ImGui::GetIO();
//	Application& app = *Application::Get();
//	io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());
//
//	// Rendering
//	ImGui::Render();
//	ImDrawData* main_draw_data = ImGui::GetDrawData();
//	const bool main_is_minimized = (main_draw_data->DisplaySize.x <= 0.0f || main_draw_data->DisplaySize.y <= 0.0f);
//	if (!main_is_minimized)
//	{
//		//VulkanRenderer::Get()->BeginImguiRenderPass(&ImguiWindowData);
//		VulkanRenderer::Get()->CommandPool->GetCommandBuffer(VulkanRenderer::Get()->CurrentBuffer)->BeginRenderPass(ImguiRenderPassHandle, ImguiFrameBuffers[VulkanRenderer::Get()->CurrentBuffer]);
//		// Record dear imgui primitives into command buffer
//		ImGui_ImplVulkan_RenderDrawData(main_draw_data, *VulkanRenderer::Get()->GetCurrentCommandBuffer()->GetCommandBufferHandle());
//		VulkanRenderer::Get()->EndRenderPass();
//	}
//
//	// Update and Render additional Platform Windows
//	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
//	{
//		GLFWwindow* backup_current_context = glfwGetCurrentContext();
//		ImGui::UpdatePlatformWindows();
//		ImGui::RenderPlatformWindowsDefault();
//		glfwMakeContextCurrent(backup_current_context);
//	}
//
//	VulkanRenderer::Get()->EndCommandBuffer();
//	// End of the current render frame and submit the frame 
//	VulkanRenderer::Get()->EndRenderFrame();
//}
//
//void VGameEditor::Shutdown()
//{
//	VulkanRenderer::Get()->GetVulkanDevice()->WaitUntilIdle();
//
//#ifdef VULKAN_GLFW
//	//ImGui_ImplVulkan_Shutdown();
//	//ImGui_ImplGlfw_Shutdown();
//	ImGui_ImplGlfw_Shutdown();
//	ImGui::DestroyContext();
//#endif
//
//	delete ImguiRenderPassHandle;
//	delete ImguiRenderLayout;
//
//	for (uint32 i = 0; i < ImguiFrameBuffers.size(); i++)
//	{
//		delete ImguiFrameBuffers[i];
//	}
//
//	delete ImguiDescriptorSetsLayout;
//	delete ImguiDescriptorPool;
//}
//
//void VGameEditor::OnEvent(WindowEvent& inWindowEvent)
//{
//	WindowEventDispatcher EventDispatcher(inWindowEvent);
//	EventDispatcher.Dispatch<WindowResizeEvent>(VE_BIND_EVENT_FUNC(OnWindowResized));
//}
//
//bool VGameEditor::OnWindowResized(WindowResizeEvent& inWindowResizeEvent)
//{
//	RenderViewportSize NewWindowSize = { inWindowResizeEvent.GetWidth(), inWindowResizeEvent.GetHeight() };
//	GameEngine->GetRenderer().Get()->OnRenderViewportResized(NewWindowSize);
//
//	ImGui_ImplVulkan_SetMinImageCount(VulkanRenderer::Get()->Swapchain->GetMinImageCount());
//	/**
//	* Recreate dear imgui frame buffers and update render area for renderpass
//	*/
//	{
//		for (uint32 i = 0; i < ImguiFrameBuffers.size(); i++)
//		{
//			ImguiFrameBuffers[i]->DestroyBuffer();
//		}
//
//		// Frame Buffers creation
//		VkExtent2D Extent = { NewWindowSize.Width, NewWindowSize.Height };
//		{
//			VulkanSwapChain const* Swapchain = VulkanRenderer::Get()->GetVulkanSwapchain();
//
//			VkImageView Attachment[1];
//
//			ImguiFrameBuffers.resize(Swapchain->GetImageCount());
//			for (uint32_t i = 0; i < Swapchain->GetImageCount(); i++)
//			{
//				Attachment[0] = Swapchain->GetSwapchainBuffer(i)->View;
//				ImguiFrameBuffers[i]->Create(1, Attachment, &Extent);
//			}
//		}
//
//		// Update render area 
//		VkRect2D RenderArea = { {0,0}, Extent };
//		ImguiRenderPassHandle->UpdateRenderArea(RenderArea);
//		ImguiRenderPassHandle->UpdateExtent2D(RenderArea.extent);
//	}
//
//	ImGuiIO& IO = ImGui::GetIO();
//	IO.DisplaySize = { (float)NewWindowSize.Width, (float)NewWindowSize.Height };
//
//	return true;
//}
