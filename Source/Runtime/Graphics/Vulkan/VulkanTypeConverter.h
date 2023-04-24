/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Core/Core.h>
#include <Runtime/Graphics/BufferGenerics.h>
#include <Runtime/Graphics/CommandBufferGenerics.h>
#include <Runtime/Graphics/Format.h>
#include <Runtime/Graphics/PipelineLayout.h>
#include <Runtime/Graphics/RenderPassGenerics.h>
#include <Runtime/Graphics/ShaderGenerics.h>
#include <Runtime/Graphics/TextureGenerics.h>

#include <string> 
#include <vulkan/vulkan_core.h>

/**
* Contains all of the conversion functions used to convert between a vulkan type to a normal type and vice versa
*/
struct VRIXIC_API VulkanTypeConverter
{
public:
    /**
    * Prints out information about a vulkan type conversion failure (Conversion from Normal to Vulkan Type)
    */
    static void ConversionFailed(const std::string& inTypeName, const std::string& inConvertToTypeName);

    /**
    * Converts a cross-api format to a vulkan format 
    */
    static VkFormat Convert(EPixelFormat inFormat);

    /**
    * Converts a vulkan format format to a cross-api 
    */
    static EPixelFormat Convert(VkFormat inFormat);

    /**
    * Converts command buffer flags into VkCommandBufferLevel
    */
    static VkCommandBufferLevel ConvertCmdBuffFlagsToVk(uint32 inFlags);

    /**
    * Converts a buffer usage flag to a vulkan buffer usage flag
    */
    static VkBufferUsageFlags ConvertBufferUsageFlagsToVk(EBufferUsageFlags inUsageFlag);

    /**
    * Converts memory flags to vulkan specific memory flags 
    */
    static VkMemoryPropertyFlags ConvertMemoryFlagsToVk(uint32 inMemoryFlags);

    /**
    * Converts sample count to vulkan sample counts 
    */
    static VkSampleCountFlagBits ConvertSampleCountToVk(uint32 inSamples);

    /**
    * Converts texture layout to vulkan image layout
    */
    static VkImageLayout ConvertTextureLayoutToVk(ETextureLayout inLayout);

    /**
    * Converts attachment descriptions to vulkan specific attachment descriptions
    */
    static VkAttachmentDescription ConvertAttachmentDescToVk(const AttachmentDescription& inDesc, VkSampleCountFlagBits inSamples);

    /**
    * Converts attachment load op to vulkan specific attachment load operation
    */
    static VkAttachmentLoadOp ConvertAttachmentLoadOpToVk(const EAttachmentLoadOp inLoadOp);

    /**
    * Converts attachment store op to vulkan specific attachment store operation
    */
    static VkAttachmentStoreOp ConvertAttachmentStoreOpToVk(const EAttachmentStoreOp inStoreOp);

    /**
    * Converts a resource type to vulkan specific descriptor type
    */
    static VkDescriptorType ConvertPipelineBDToVk(const PipelineBindingDescriptor inDesc);

    /**
    * Converts a shader flags to vulkan specific shader flags
    */
    static VkShaderStageFlags ConvertShaderFlagsToVk(uint32 inFlags);

    /**
    * Converts primitive topology to vulkan specific primitive topology
    */
    static VkPrimitiveTopology ConvertTopologyToVk(EPrimitiveTopology inTopology);

    /**
    * Converts render viewport to vulkan specific viewport
    */
    static VkViewport ConvertViewportToVk(const RenderViewport& inViewport);

    /**
    * Converts render scissor to vulkan specific scissor (VkRect2D)
    */
    static VkRect2D ConvertScissorToVk(const RenderScissor& inViewport);

    /**
    * Converts polygon mode to vulkan specific polygon mode
    */
    static VkPolygonMode ConvertPolygonModeToVk(EPolygonMode inMode);

    /**
    * Converts cull mode to vulkan specific cull mode flags
    */
    static VkCullModeFlags ConvertCullModeToVk(ECullMode inMode);

    /**
    * Converts front face to vulkan specific front face
    */
    static VkFrontFace ConvertFrontFaceToVk(EFrontFace inFace);

    /**
    * Converts stencil op to vulkan specific stencil op
    */
    static VkStencilOp ConvertStencilOpToVk(EStencilOp inOp);

    /**
    * Converts compare op to vulkan specific compare op
    */
    static VkCompareOp ConvertCompareOpToVk(ECompareOp inOp);

    /**
    * Converts blend factor to vulkan specific blend factor
    */
    static VkBlendFactor ConvertBlendFactorToVk(EBlendFactor inFactor);

    /**
    * Converts blend op to vulkan specific blend op
    */
    static VkBlendOp ConvertBlendOpToVk(EBlendOp inOp);

    /**
    * Converts color component mask to vulkan specific color component flags
    */
    static VkColorComponentFlags ConvertColorComponentMaskToVk(uint8 inColorMask);

    /**
    * Converts logic op to vulkan specific logic op
    */
    static VkLogicOp ConvertLogicOpToVk(ELogicOp inOp);

    /**
    * Converts shader type to vulkan specific shader stage flages 
    */
    static VkShaderStageFlagBits ConvertShaderTypeToVk(EShaderType inType);

    /**
    * Converts texture type to vulkan specific texture type (image type)
    */
    static VkImageType ConvertTextureTypeToVk(ETextureType inType);

    /**
    * Converts texture flags to vulkan specific texture flags (image usage flags)
    */
    static VkImageUsageFlags ConvertTextureUsageFlagsToVk(uint32 inFlags);

    /**
    * Converts texture type to vulkan specific texture view type (image type)
    */
    static VkImageViewType ConvertTextureViewTypeToVk(ETextureType inType);

    /**
    * Converts subpass access flags to vulkan specific access flags
    */
    static VkAccessFlags ConvertSubpassAccessFlagsToVk(uint32 inFlags);

    /**
    * Converts SubpassDependencyDescription to vulkan specific VkSubpassDependency
    */
    static VkSubpassDependency ConvertSubpassDependencyDescToVk(const SubpassDependencyDescription& inDesc);
};
