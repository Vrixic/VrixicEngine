#pragma once

#include <cassert>
#include <stdexcept>
#include <vector>
#include <Windows.h>
#include "vulkan/vulkan.h"
#include <vulkan/vulkan_win32.h>
#include <iostream>
#include <Misc/Defines/GenericDefines.h>

/* The # macro will turn the expression f into a string literal */
#define VK_CHECK_RESULT(f)																				\
{																										\
	VkResult res = (f);																					\
	if (res != VK_SUCCESS)																				\
	{																									\
		std::cout << "Fatal : \"" << #f << "\" in " << __FILE__ << " at line " << __LINE__ << "\n"; \
		ASSERT(false);																		\
	}																									\
}

namespace VulkanUtils
{
	/* Descriptions for vulkan pipeline, and create infos... */
	namespace Descriptions
	{
		struct VertexAttribute
		{
			uint32 Location;
			uint32 Binding;
			uint32 Format;
			uint32 Offset;

			void WriteTo(VkVertexInputAttributeDescription& outVertexAttribute) const
			{
				outVertexAttribute.location = Location;
				outVertexAttribute.binding = Binding;
				outVertexAttribute.format = (VkFormat)Format;
				outVertexAttribute.offset = Offset;
			}
		};

		struct VertexBinding
		{
			uint32 Binding;
			uint32 Stride;
			uint32 InputRate;

			void WriteTo(VkVertexInputBindingDescription& outVertexBinding) const
			{
				outVertexBinding.binding = Binding;
				outVertexBinding.stride = Stride;
				outVertexBinding.inputRate = (VkVertexInputRate)InputRate;
			}
		};

		struct Rasterizer
		{
			uint32 RasterizerDiscardEnable;
			uint32 PolygonMode;
			float LineWidth;
			uint32 CullMode;
			uint32 FrontFace;
			uint32 DepthClampEnable;
			uint32 DepthBiasEnable;
			float DepthBiasClamp;
			float DepthBiasConstantFactor;
			float DepthBiasSlopeFactor;

			void WriteTo(VkPipelineRasterizationStateCreateInfo& outRasterizer) const
			{
				outRasterizer.rasterizerDiscardEnable = RasterizerDiscardEnable;
				outRasterizer.polygonMode = (VkPolygonMode)PolygonMode;
				outRasterizer.lineWidth = LineWidth;
				outRasterizer.cullMode = CullMode;
				outRasterizer.frontFace = (VkFrontFace)FrontFace;
				outRasterizer.depthClampEnable = DepthClampEnable;
				outRasterizer.depthBiasEnable = DepthBiasEnable;
				outRasterizer.depthBiasClamp = DepthBiasClamp;
				outRasterizer.depthBiasConstantFactor = DepthBiasConstantFactor;
				outRasterizer.depthBiasSlopeFactor = DepthBiasSlopeFactor;
			}
		};

		struct AttachmentReference
		{
			uint32 Attachement;
			uint32 Layout;

			void WriteTo(VkAttachmentReference& outAttachmentReference) const
			{
				outAttachmentReference.attachment = Attachement;
				outAttachmentReference.layout = (VkImageLayout)Layout;
			}
		};

		struct AttachmentDescription
		{
			uint32 Format;
			uint32 Samples;
			uint32 LoadOp;
			uint32 StoreOp;
			uint32 StencilLoadOp;
			uint32 StencilStoreOp;
			uint32 InitialLayout;
			uint32 FinalLayout;

			void WriteTo(VkAttachmentDescription& outAttachmentDescription) const
			{
				outAttachmentDescription.format = (VkFormat)Format;
				outAttachmentDescription.samples = (VkSampleCountFlagBits)(Samples);
				outAttachmentDescription.loadOp = (VkAttachmentLoadOp)LoadOp;
				outAttachmentDescription.storeOp = (VkAttachmentStoreOp)StoreOp;
				outAttachmentDescription.stencilLoadOp = (VkAttachmentLoadOp)StencilLoadOp;
				outAttachmentDescription.stencilStoreOp = (VkAttachmentStoreOp)StencilStoreOp;
				outAttachmentDescription.initialLayout = (VkImageLayout)InitialLayout;
				outAttachmentDescription.finalLayout = (VkImageLayout)FinalLayout;
			}
		};

		struct DescriptorSetLayoutBinding
		{
			uint32 Binding;
			uint32 DescriptorType;
			uint32 DescriptorCount;
			uint32 StageFlags;

			void WriteTo(VkDescriptorSetLayoutBinding& outDescriptorSetLayoutBinding) const
			{
				outDescriptorSetLayoutBinding.binding = Binding;
				outDescriptorSetLayoutBinding.descriptorType = (VkDescriptorType)DescriptorType;
				outDescriptorSetLayoutBinding.descriptorCount = DescriptorCount;
				outDescriptorSetLayoutBinding.stageFlags = StageFlags;
			}
		};

		struct WriteDescriptorSet
		{
			uint32 DstBinding;
			VkDescriptorSet DstSet;
			uint32 DstArrayElement;
			uint32 DescriptorCount;
			VkDescriptorType DescriptorType;

			void WriteTo(VkWriteDescriptorSet& outWriteDescriptorSet) const
			{
				outWriteDescriptorSet.dstBinding = DstBinding;
				outWriteDescriptorSet.dstSet = DstSet;
				outWriteDescriptorSet.dstArrayElement = DstArrayElement;
				outWriteDescriptorSet.descriptorCount = DescriptorCount;
				outWriteDescriptorSet.descriptorType = DescriptorType;
			}
		};

		struct DescriptorSetLayoutCreateInfo
		{
			VkDescriptorSetLayoutCreateFlags Flags;

			void WriteTo(VkDescriptorSetLayoutCreateInfo& outDescriptorSetLayoutCreateInfo) const
			{
				outDescriptorSetLayoutCreateInfo.flags = Flags;
			}
		};

		struct VulkanBufferCreateInfo
		{
			VkBufferUsageFlags BufferUsageFlags;
			VkMemoryPropertyFlags MemoryPropertyFlags;
			VkDeviceSize DeviceSize;
		};
	}

	/* Helpers to reduce vulkans verbosity */
	namespace Helpers
	{
		/* Aka. GetBestGPU() */
		inline VkResult GetBestPhysicalDevice(const VkPhysicalDevice* inPhysicalDevices, unsigned int inPhysicalDevicesCount, VkPhysicalDevice& outBestPhysicalDevice)
		{
			VkResult Result = VK_SUCCESS;

			VkPhysicalDeviceProperties PhysicalDeviceProp = {};

			// Best Type[Discrete, Virtual, Integrated, CPU / OTHER] in order
			unsigned int BestDeviceTypeIndex = 0;
			unsigned int BestDeviceType = UINT32_MAX;
			unsigned int CurrentDeviceType = 0;

			for (unsigned int i = 0; i < inPhysicalDevicesCount; ++i)
			{
				vkGetPhysicalDeviceProperties(inPhysicalDevices[i], &PhysicalDeviceProp);

				switch (PhysicalDeviceProp.deviceType)
				{
				case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
					CurrentDeviceType = 0;
					break;
				case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
					CurrentDeviceType = 1;
					break;
				case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
					CurrentDeviceType = 2;
					break;
				default:
					CurrentDeviceType = 3;
					break;
				}

				if (BestDeviceType > CurrentDeviceType)
				{
					BestDeviceType = CurrentDeviceType;
					BestDeviceTypeIndex = i;
				}
			}

			outBestPhysicalDevice = inPhysicalDevices[BestDeviceTypeIndex];

			return Result;
		}

		inline uint32 GetQueueFamilyIndex(std::vector<VkQueueFamilyProperties>& inQueueFamilyProperties, VkQueueFlags inQueueFlags)
		{
			// Dedicated queue for Compute
			// Try to find a queue family index that supports Compute but not Graphics
			if ((inQueueFlags & VK_QUEUE_COMPUTE_BIT) == inQueueFlags)
			{
				for (uint32 i = 0; i < static_cast<uint32>(inQueueFamilyProperties.size()); i++)
				{
					if ((inQueueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) && ((inQueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
					{
						return i;
					}
				}
			}

			// Dedicated queue for Transfer
			// Try to find a queue family index that supports Transfer but not Graphics and Compute
			if ((inQueueFlags & VK_QUEUE_TRANSFER_BIT) == inQueueFlags)
			{
				for (uint32 i = 0; i < static_cast<uint32>(inQueueFamilyProperties.size()); i++)
				{
					if ((inQueueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT) && ((inQueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) && ((inQueueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0))
					{
						return i;
					}
				}
			}

			// For other queue types or if no separate Compute queue is present, return the first one to support the requested flags
			for (uint32 i = 0; i < static_cast<uint32>(inQueueFamilyProperties.size()); i++)
			{
				if ((inQueueFamilyProperties[i].queueFlags & inQueueFlags) == inQueueFlags)
				{
					return i;
				}
			}

			throw std::runtime_error("Could not find a matching queue family index");
		}

		inline bool ExtensionSupported(std::string inExtension, std::vector<std::string> inSupportedExtensions)
		{
			return (std::find(inSupportedExtensions.begin(), inSupportedExtensions.end(), inExtension) != inSupportedExtensions.end());
		}

		inline VkBool32 GetSupportedDepthFormat(VkPhysicalDevice inPhysicalDevice, VkFormat* outDepthFormat)
		{
			// Since all depth formats may be optional, we need to find a suitable depth format to use
			// Start with the highest precision packed format
			std::vector<VkFormat> DepthFormats = {
				VK_FORMAT_D32_SFLOAT_S8_UINT,
				VK_FORMAT_D32_SFLOAT,
				VK_FORMAT_D24_UNORM_S8_UINT,
				VK_FORMAT_D16_UNORM_S8_UINT,
				VK_FORMAT_D16_UNORM
			};

			for (uint32 i = 0; i < DepthFormats.size(); ++i)
			{
				VkFormatProperties FormatProps = { };
				vkGetPhysicalDeviceFormatProperties(inPhysicalDevice, DepthFormats[i], &FormatProps);

				// Format must support depth stencil attachment for optimal tiling
				if (FormatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
				{
					*outDepthFormat = DepthFormats[i];
					return true;
				}
			}

			return false;
		}
	}

	/* Create Infos */
	namespace Initializers
	{
		inline VkApplicationInfo ApplicationInfo()
		{
			VkApplicationInfo ApplicationInfo = { };
			ApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;

			return ApplicationInfo;
		}

		inline VkInstanceCreateInfo InstanceCreateInfo()
		{
			VkInstanceCreateInfo InstanceCreateInfo = { };
			InstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

			return InstanceCreateInfo;
		}

		inline VkFramebufferCreateInfo FrameBufferCreateInfo()
		{
			VkFramebufferCreateInfo FrameBufferCreateInfo = { };
			FrameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;

			return FrameBufferCreateInfo;
		}

		inline VkImageCreateInfo ImageCreateInfo()
		{
			VkImageCreateInfo ImageCreateInfo = { };
			ImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;

			return ImageCreateInfo;
		}

		inline VkImageViewCreateInfo ImageViewCreateInfo()
		{
			VkImageViewCreateInfo ImageViewCreateInfo = { };
			ImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

			return ImageViewCreateInfo;
		}

		inline VkMemoryAllocateInfo MemoryAllocateInfo()
		{
			VkMemoryAllocateInfo MemoryAllocateInfo = { };
			MemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

			return MemoryAllocateInfo;
		}

		inline VkDeviceQueueCreateInfo DeviceQueueCreateInfo(uint32 inQueueFamilyIndex, uint32 inQueueCount, const float* inPtrQueuePriorities)
		{
			VkDeviceQueueCreateInfo DeviceQueueCreateInfo = { };
			DeviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			DeviceQueueCreateInfo.queueFamilyIndex = inQueueFamilyIndex;
			DeviceQueueCreateInfo.queueCount = inQueueCount;
			DeviceQueueCreateInfo.pQueuePriorities = inPtrQueuePriorities;

			return DeviceQueueCreateInfo;
		}

		inline VkDeviceCreateInfo DeviceCreateInfo()
		{
			VkDeviceCreateInfo DeviceCreateInfo = { };
			DeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

			return DeviceCreateInfo;
		}

		inline VkSubmitInfo SubmitInfo()
		{
			VkSubmitInfo SubmitInfo = { };
			SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

			return SubmitInfo;
		}

		inline VkWin32SurfaceCreateInfoKHR Win32SurfaceCreateInfoKHR(HINSTANCE* inWindowInstance, HWND* inWindow)
		{
			VkWin32SurfaceCreateInfoKHR Win32SurfaceCreateInfoKHR = { };
			Win32SurfaceCreateInfoKHR.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
			Win32SurfaceCreateInfoKHR.hinstance = *inWindowInstance;
			Win32SurfaceCreateInfoKHR.hwnd = *inWindow;

			return Win32SurfaceCreateInfoKHR;
		}

		inline VkSwapchainCreateInfoKHR SwapchainCreateInfoKHR()
		{
			VkSwapchainCreateInfoKHR SwapchainCreateInfoKHR = { };
			SwapchainCreateInfoKHR.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;

			return SwapchainCreateInfoKHR;
		}

		inline VkPresentInfoKHR PresentInfoKHR()
		{
			VkPresentInfoKHR PresentInfoKHR = { };
			PresentInfoKHR.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

			return PresentInfoKHR;
		}

		inline VkFenceCreateInfo FenceCreateInfo(VkFenceCreateFlags inFenceCreateFlags, const void* inPtrNext)
		{
			VkFenceCreateInfo FenceCreateInfo = { };
			FenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			FenceCreateInfo.flags = inFenceCreateFlags;
			FenceCreateInfo.pNext = inPtrNext;

			return FenceCreateInfo;
		}

		inline VkSemaphoreCreateInfo SemaphoreCreateInfo(const void* inPtrNext)
		{
			VkSemaphoreCreateInfo SemaphoreCreateInfo = { };
			SemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			SemaphoreCreateInfo.pNext = inPtrNext;

			return SemaphoreCreateInfo;
		}

		inline VkCommandBufferAllocateInfo CommandBufferAllocateInfo()
		{
			VkCommandBufferAllocateInfo CommandBufferAllocateInfo = { };
			CommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;

			return CommandBufferAllocateInfo;
		}

		inline VkCommandBufferBeginInfo CommandBufferBeginInfo(const void* inPtrNext)
		{
			VkCommandBufferBeginInfo CommandBufferBeginInfo = { };
			CommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			CommandBufferBeginInfo.pNext = inPtrNext;

			return CommandBufferBeginInfo;
		}

		inline VkRenderPassBeginInfo RenderPassBeginInfo(const VkRenderPass* inRenderPass, const void* inPtrNext)
		{
			VkRenderPassBeginInfo RenderPassBeginInfo = { };
			RenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			RenderPassBeginInfo.renderPass = *inRenderPass;
			RenderPassBeginInfo.pNext = inPtrNext;

			return RenderPassBeginInfo;
		}

		inline VkRenderPassCreateInfo RenderPassCreateInfo()
		{
			VkRenderPassCreateInfo RenderPassCreateInfo = { };
			RenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

			return RenderPassCreateInfo;
		}

		inline VkCommandPoolCreateInfo CommandPoolCreateInfo(VkCommandPoolCreateFlags inFlags, uint32 inQueueFamilyIndex, const void* inPtrNext)
		{
			VkCommandPoolCreateInfo CommandPoolCreateInfo = { };
			CommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			CommandPoolCreateInfo.queueFamilyIndex = inQueueFamilyIndex;
			CommandPoolCreateInfo.flags = inFlags;
			CommandPoolCreateInfo.pNext = inPtrNext;

			return CommandPoolCreateInfo;
		}

		inline VkShaderModuleCreateInfo ShaderModuleCreateInfo()
		{
			VkShaderModuleCreateInfo ShaderModuleCreateInfo = { };
			ShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

			return ShaderModuleCreateInfo;
		}

		inline VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo()
		{
			VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo = { };
			DescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;

			return DescriptorSetAllocateInfo;
		}

		inline VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo()
		{
			VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo = { };
			DescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

			return DescriptorSetLayoutCreateInfo;
		}

		inline VkBufferCreateInfo BufferCreateInfo()
		{
			VkBufferCreateInfo BufferCreateInfo = { };
			BufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;

			return BufferCreateInfo;
		}

		inline VkWriteDescriptorSet WriteDescriptorSet()
		{
			VkWriteDescriptorSet WriteDescriptorSet = { };
			WriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;

			return WriteDescriptorSet;
		}

		inline VkDescriptorPoolCreateInfo DescriptorPoolCreateInfo()
		{
			VkDescriptorPoolCreateInfo DescriptorPoolCreateInfo = { };
			DescriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;

			return DescriptorPoolCreateInfo;
		}

		inline VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo()
		{
			VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo = { };
			PipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

			return PipelineLayoutCreateInfo;
		}

		inline VkGraphicsPipelineCreateInfo GraphicsPipelineCreateInfo()
		{
			VkGraphicsPipelineCreateInfo GraphicsPipelineCreateInfo = { };
			GraphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

			return GraphicsPipelineCreateInfo;
		}

		inline VkPipelineCacheCreateInfo PipelineCacheCreateInfo()
		{
			VkPipelineCacheCreateInfo PipelineCacheCreateInfo = { };
			PipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

			return PipelineCacheCreateInfo;
		}

		inline VkPipelineShaderStageCreateInfo PipelineShaderStageCreateInfo()
		{
			VkPipelineShaderStageCreateInfo PipelineShaderStageCreateInfo = { };
			PipelineShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

			return PipelineShaderStageCreateInfo;
		}

		inline VkPipelineInputAssemblyStateCreateInfo PipelineInputAssemblyStateCreateInfo()
		{
			VkPipelineInputAssemblyStateCreateInfo PipelineInputAssemblyStateCreateInfo = { };
			PipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;

			return PipelineInputAssemblyStateCreateInfo;
		}

		inline VkPipelineVertexInputStateCreateInfo PipelineVertexInputStateCreateInfo()
		{
			VkPipelineVertexInputStateCreateInfo PipelineVertexInputStateCreateInfo = { };
			PipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

			return PipelineVertexInputStateCreateInfo;
		}

		inline VkPipelineViewportStateCreateInfo PipelineViewportStateCreateInfo()
		{
			VkPipelineViewportStateCreateInfo PipelineViewportStateCreateInfo = { };
			PipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;

			return PipelineViewportStateCreateInfo;
		}

		inline VkPipelineRasterizationStateCreateInfo PipelineRasterizationStateCreateInfo()
		{
			VkPipelineRasterizationStateCreateInfo PipelineRasterizationStateCreateInfo = { };
			PipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;

			return PipelineRasterizationStateCreateInfo;
		}

		inline VkPipelineMultisampleStateCreateInfo PipelineMultisampleStateCreateInfo()
		{
			VkPipelineMultisampleStateCreateInfo PipelineMultisampleStateCreateInfo = { };
			PipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;

			return PipelineMultisampleStateCreateInfo;
		}

		inline VkPipelineDepthStencilStateCreateInfo PipelineDepthStencilStateCreateInfo()
		{
			VkPipelineDepthStencilStateCreateInfo PipelineDepthStencilStateCreateInfo = { };
			PipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

			return PipelineDepthStencilStateCreateInfo;
		}

		inline VkPipelineColorBlendStateCreateInfo PipelineColorBlendStateCreateInfo()
		{
			VkPipelineColorBlendStateCreateInfo PipelineColorBlendStateCreateInfo = { };
			PipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

			return PipelineColorBlendStateCreateInfo;
		}

		inline VkPipelineDynamicStateCreateInfo PipelineDynamicStateCreateInfo()
		{
			VkPipelineDynamicStateCreateInfo PipelineDynamicStateCreateInfo = { };
			PipelineDynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;

			return PipelineDynamicStateCreateInfo;
		}
	}
}

