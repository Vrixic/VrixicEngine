/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#include "VulkanTypeConverter.h"

#include <Misc/Assert.h>
#include <Misc/Defines/StringDefines.h>

void VulkanTypeConverter::ConversionFailed(const std::string& inTypeName, const std::string& inConvertToTypeName)
{
    VE_ASSERT(false, VE_TEXT("Failed to convert {0} to {1}!!"), inTypeName, inConvertToTypeName);
}

VkFormat VulkanTypeConverter::Convert(EPixelFormat inFormat)
{
    switch (inFormat)
    {
    case EPixelFormat::Undefined:           return VK_FORMAT_UNDEFINED;

        /* Alpha Channels */
    case EPixelFormat::A8UNorm:             break; // not supported 

        /* Red Channel Color Formats 8-bit */
    case EPixelFormat::R8UNorm:             return VK_FORMAT_R8_UNORM;
    case EPixelFormat::R8SNorm:             return VK_FORMAT_R8_SNORM;
    case EPixelFormat::R8UInt:              return VK_FORMAT_R8_UINT;
    case EPixelFormat::R8SInt:              return VK_FORMAT_R8_SINT;
    case EPixelFormat::R8SRGB:              return VK_FORMAT_R8_SRGB;

        /* Red Channel Color Formats 16-bit */
    case EPixelFormat::R16UNorm:            return VK_FORMAT_R16_UNORM;
    case EPixelFormat::R16SNorm:            return VK_FORMAT_R16_SNORM;
    case EPixelFormat::R16UInt:             return VK_FORMAT_R16_UINT;
    case EPixelFormat::R16SInt:             return VK_FORMAT_R16_SINT;
    case EPixelFormat::R16Float:            return VK_FORMAT_R16_SFLOAT;

        /* Red Channel Color Formats 32-bit */
    case EPixelFormat::R32UInt:             return VK_FORMAT_R32_UINT;
    case EPixelFormat::R32SInt:             return VK_FORMAT_R32_SINT;
    case EPixelFormat::R32Float:            return VK_FORMAT_R32_SFLOAT;

        /* Red Channel Color Formats 64-bit */
    case EPixelFormat::R64Float:            return VK_FORMAT_R64_SFLOAT;

        /* Red and Blue Channel Color Formats 8-bit */
    case EPixelFormat::RG8UNorm:            return VK_FORMAT_R8G8_UNORM;
    case EPixelFormat::RG8SNorm:            return VK_FORMAT_R8G8_SNORM;
    case EPixelFormat::RG8UInt:             return VK_FORMAT_R8G8_UINT;
    case EPixelFormat::RG8SInt:             return VK_FORMAT_R8G8_SINT;

        /* Red and Blue Channel Color Formats 16-bit */
    case EPixelFormat::RG16UNorm:           return VK_FORMAT_R16G16_UNORM;
    case EPixelFormat::RG16SNorm:           return VK_FORMAT_R16G16_SNORM;
    case EPixelFormat::RG16UInt:            return VK_FORMAT_R16G16_UINT;
    case EPixelFormat::RG16SInt:            return VK_FORMAT_R16G16_SINT;
    case EPixelFormat::RG16Float:           return VK_FORMAT_R16G16_SFLOAT;

        /* Red and Blue Channel Color Formats 32-bit */
    case EPixelFormat::RG32UInt:            return VK_FORMAT_R32G32_UINT;
    case EPixelFormat::RG32SInt:            return VK_FORMAT_R32G32_SINT;
    case EPixelFormat::RG32Float:           return VK_FORMAT_R32G32_SFLOAT;

        /* Red and Blue Channel Color Formats 64-bit */
    case EPixelFormat::RG64Float:           return VK_FORMAT_R64G64_SFLOAT;

        /* Red, Blue and Green Channel Color Formats 8-bit */
    case EPixelFormat::RGB8UNorm:           return VK_FORMAT_R8G8B8_UNORM;
    case EPixelFormat::RGB8UNorm_sRGB:      return VK_FORMAT_R8G8B8_SRGB;
    case EPixelFormat::RGB8SNorm:           return VK_FORMAT_R8G8B8_SNORM;
    case EPixelFormat::RGB8UInt:            return VK_FORMAT_R8G8B8_UINT;
    case EPixelFormat::RGB8SInt:            return VK_FORMAT_R8G8B8_SINT;

        /* Red, Blue and Green Channel Color Formats 16-bit */
    case EPixelFormat::RGB16UNorm:          return VK_FORMAT_R16G16B16_UNORM;
    case EPixelFormat::RGB16SNorm:          return VK_FORMAT_R16G16B16_SNORM;
    case EPixelFormat::RGB16UInt:           return VK_FORMAT_R16G16B16_UINT;
    case EPixelFormat::RGB16SInt:           return VK_FORMAT_R16G16B16_SINT;
    case EPixelFormat::RGB16Float:          return VK_FORMAT_R16G16B16_SFLOAT;

        /* Red, Blue and Green Channel Color Formats 32-bit */
    case EPixelFormat::RGB32UInt:           return VK_FORMAT_R32G32B32_UINT;
    case EPixelFormat::RGB32SInt:           return VK_FORMAT_R32G32B32_SINT;
    case EPixelFormat::RGB32Float:          return VK_FORMAT_R32G32B32_SFLOAT;

        /* Red, Blue and Green Channel Color Formats 64-bit */
    case EPixelFormat::RGB64Float:          return VK_FORMAT_R64G64B64_SFLOAT;

        /* Red, Blue, Green and Alpha Channel Color Formats 8-bit */
    case EPixelFormat::RGBA8UNorm:          return VK_FORMAT_R8G8B8A8_UNORM;
    case EPixelFormat::RGBA8UNorm_sRGB:     return VK_FORMAT_R8G8B8A8_SRGB;
    case EPixelFormat::RGBA8SNorm:          return VK_FORMAT_R8G8B8A8_SNORM;
    case EPixelFormat::RGBA8UInt:           return VK_FORMAT_R8G8B8A8_UINT;
    case EPixelFormat::RGBA8SInt:           return VK_FORMAT_R8G8B8A8_SINT;

        /* Red, Blue, Green and Alpha Channel Color Formats 16-bit */
    case EPixelFormat::RGBA16UNorm:         return VK_FORMAT_R16G16B16A16_UNORM;
    case EPixelFormat::RGBA16SNorm:         return VK_FORMAT_R16G16B16A16_SNORM;
    case EPixelFormat::RGBA16UInt:          return VK_FORMAT_R16G16B16A16_UINT;
    case EPixelFormat::RGBA16SInt:          return VK_FORMAT_R16G16B16A16_SINT;
    case EPixelFormat::RGBA16Float:         return VK_FORMAT_R16G16B16A16_SFLOAT;

        /* Red, Blue, Green and Alpha Channel Color Formats 32-bit */
    case EPixelFormat::RGBA32UInt:          return VK_FORMAT_R32G32B32A32_UINT;
    case EPixelFormat::RGBA32SInt:          return VK_FORMAT_R32G32B32A32_SINT;
    case EPixelFormat::RGBA32Float:         return VK_FORMAT_R32G32B32A32_SFLOAT;

        /* Red, Blue, Green and Alpha Channel Color Formats 64-bit */
    case EPixelFormat::RGBA64Float:         return VK_FORMAT_R64G64B64A64_SFLOAT;

        /* Blue, Green, Red, and Alpha Channel Color Formats 8-bit */
    case EPixelFormat::BGRA8UNorm:          return VK_FORMAT_B8G8R8A8_UNORM;
    case EPixelFormat::BGRA8UNorm_sRGB:     return VK_FORMAT_B8G8R8A8_SRGB;
    case EPixelFormat::BGRA8SNorm:          return VK_FORMAT_B8G8R8A8_SNORM;
    case EPixelFormat::BGRA8UInt:           return VK_FORMAT_B8G8R8A8_UINT;
    case EPixelFormat::BGRA8SInt:           return VK_FORMAT_B8G8R8A8_SINT;

        /* Depth Stencil Formats */
    case EPixelFormat::D16UNorm:            return VK_FORMAT_D16_UNORM;
    case EPixelFormat::D24UNormS8UInt:      return VK_FORMAT_D24_UNORM_S8_UINT;
    case EPixelFormat::D32Float:            return VK_FORMAT_D32_SFLOAT;
    case EPixelFormat::D32FloatS8X24UInt:   return VK_FORMAT_D32_SFLOAT_S8_UINT;

    case EPixelFormat::S8UInt:              return VK_FORMAT_S8_UINT;

    }

    ConversionFailed("EPixelFormat", "VkFormat");

    return VK_FORMAT_UNDEFINED;
}

EPixelFormat VulkanTypeConverter::Convert(VkFormat inFormat)
{
    switch (inFormat)
    {
    case VK_FORMAT_UNDEFINED:               return EPixelFormat::Undefined;

        /* Red Channel Color Formats 8-bit */
    case VK_FORMAT_R8_UNORM:                return EPixelFormat::R8UNorm;
    case VK_FORMAT_R8_SNORM:                return EPixelFormat::R8SNorm;
    case VK_FORMAT_R8_UINT:                 return EPixelFormat::R8UInt;
    case VK_FORMAT_R8_SINT:                 return EPixelFormat::R8SInt;
    case VK_FORMAT_R8_SRGB:                 return EPixelFormat::R8SRGB;

        /* Red Channel Color Formats 16-bit */
    case VK_FORMAT_R16_UNORM:               return EPixelFormat::R16UNorm;
    case VK_FORMAT_R16_SNORM:               return EPixelFormat::R16SNorm;
    case VK_FORMAT_R16_UINT:                return EPixelFormat::R16UInt;
    case VK_FORMAT_R16_SINT:                return EPixelFormat::R16SInt;
    case VK_FORMAT_R16_SFLOAT:              return EPixelFormat::R16Float;

        /* Red Channel Color Formats 32-bit */
    case VK_FORMAT_R32_UINT:                return EPixelFormat::R32UInt;
    case VK_FORMAT_R32_SINT:                return EPixelFormat::R32SInt;
    case VK_FORMAT_R32_SFLOAT:              return EPixelFormat::R32Float;

        /* Red Channel Color Formats 64-bit */
    case VK_FORMAT_R64_SFLOAT:              return EPixelFormat::R64Float;

        /* Red and Blue Channel Color Formats 8-bit */
    case VK_FORMAT_R8G8_UNORM:              return EPixelFormat::RG8UNorm;
    case VK_FORMAT_R8G8_SNORM:              return EPixelFormat::RG8SNorm;
    case VK_FORMAT_R8G8_UINT:               return EPixelFormat::RG8UInt;
    case VK_FORMAT_R8G8_SINT:               return EPixelFormat::RG8SInt;

        /* Red and Blue Channel Color Formats 16-bit */
    case VK_FORMAT_R16G16_UNORM:            return EPixelFormat::RG16UNorm;
    case VK_FORMAT_R16G16_SNORM:            return EPixelFormat::RG16SNorm;
    case VK_FORMAT_R16G16_UINT:             return EPixelFormat::RG16UInt;
    case VK_FORMAT_R16G16_SINT:             return EPixelFormat::RG16SInt;
    case VK_FORMAT_R16G16_SFLOAT:           return EPixelFormat::RG16Float;

        /* Red and Blue Channel Color Formats 32-bit */
    case VK_FORMAT_R32G32_UINT:             return EPixelFormat::RG32UInt;
    case VK_FORMAT_R32G32_SINT:             return EPixelFormat::RG32SInt;
    case VK_FORMAT_R32G32_SFLOAT:           return EPixelFormat::RG32Float;

        /* Red and Blue Channel Color Formats 64-bit */
    case VK_FORMAT_R64G64_SFLOAT:           return EPixelFormat::RG64Float;

        /* Red, Blue and Green Channel Color Formats 8-bit */
    case VK_FORMAT_R8G8B8_UNORM:            return EPixelFormat::RGB8UNorm;
    case VK_FORMAT_R8G8B8_SRGB:             return EPixelFormat::RGB8UNorm_sRGB;
    case VK_FORMAT_R8G8B8_SNORM:            return EPixelFormat::RGB8SNorm;
    case VK_FORMAT_R8G8B8_UINT:             return EPixelFormat::RGB8UInt;
    case VK_FORMAT_R8G8B8_SINT:             return EPixelFormat::RGB8SInt;

        /* Red, Blue and Green Channel Color Formats 16-bit */
    case VK_FORMAT_R16G16B16_UNORM:          return EPixelFormat::RGB16UNorm;
    case VK_FORMAT_R16G16B16_SNORM:          return EPixelFormat::RGB16SNorm;
    case VK_FORMAT_R16G16B16_UINT:           return EPixelFormat::RGB16UInt;
    case VK_FORMAT_R16G16B16_SINT:           return EPixelFormat::RGB16SInt;
    case VK_FORMAT_R16G16B16_SFLOAT:         return EPixelFormat::RGB16Float;

        /* Red, Blue and Green Channel Color Formats 32-bit */
    case VK_FORMAT_R32G32B32_UINT:           return EPixelFormat::RGB32UInt;
    case VK_FORMAT_R32G32B32_SINT:           return EPixelFormat::RGB32SInt;
    case VK_FORMAT_R32G32B32_SFLOAT:         return EPixelFormat::RGB32Float;

        /* Red, Blue and Green Channel Color Formats 64-bit */
    case VK_FORMAT_R64G64B64_SFLOAT:         return EPixelFormat::RGB64Float;

        /* Red, Blue, Green and Alpha Channel Color Formats 8-bit */
    case VK_FORMAT_R8G8B8A8_UNORM:              return EPixelFormat::RGBA8UNorm;
    case VK_FORMAT_R8G8B8A8_SRGB:              return EPixelFormat::RGBA8UNorm_sRGB;
    case VK_FORMAT_R8G8B8A8_SNORM:              return EPixelFormat::RGBA8SNorm;
    case VK_FORMAT_R8G8B8A8_UINT:               return EPixelFormat::RGBA8UInt;
    case VK_FORMAT_R8G8B8A8_SINT:               return EPixelFormat::RGBA8SInt;

        /* Red, Blue, Green and Alpha Channel Color Formats 16-bit */
    case VK_FORMAT_R16G16B16A16_UNORM:          return EPixelFormat::RGBA16UNorm;
    case VK_FORMAT_R16G16B16A16_SNORM:          return EPixelFormat::RGBA16SNorm;
    case VK_FORMAT_R16G16B16A16_UINT:           return EPixelFormat::RGBA16UInt;
    case VK_FORMAT_R16G16B16A16_SINT:           return EPixelFormat::RGBA16SInt;
    case VK_FORMAT_R16G16B16A16_SFLOAT:         return EPixelFormat::RGBA16Float;

        /* Red, Blue, Green and Alpha Channel Color Formats 32-bit */
    case VK_FORMAT_R32G32B32A32_UINT:           return EPixelFormat::RGBA32UInt;
    case VK_FORMAT_R32G32B32A32_SINT:           return EPixelFormat::RGBA32SInt;
    case VK_FORMAT_R32G32B32A32_SFLOAT:         return EPixelFormat::RGBA32Float;

        /* Red, Blue, Green and Alpha Channel Color Formats 64-bit */
    case VK_FORMAT_R64G64B64A64_SFLOAT:         return EPixelFormat::RGBA64Float;

        /* Blue, Green, Red, and Alpha Channel Color Formats 8-bit */
    case VK_FORMAT_B8G8R8A8_UNORM:              return EPixelFormat::BGRA8UNorm;
    case VK_FORMAT_B8G8R8A8_SRGB:               return EPixelFormat::BGRA8UNorm_sRGB;
    case VK_FORMAT_B8G8R8A8_SNORM:              return EPixelFormat::BGRA8SNorm;
    case VK_FORMAT_B8G8R8A8_UINT:               return EPixelFormat::BGRA8UInt;
    case VK_FORMAT_B8G8R8A8_SINT:               return EPixelFormat::BGRA8SInt;

        /* Depth Stencil Formats */
    case VK_FORMAT_D16_UNORM:                   return EPixelFormat::D16UNorm;
    case VK_FORMAT_D24_UNORM_S8_UINT:           return EPixelFormat::D24UNormS8UInt;
    case VK_FORMAT_D32_SFLOAT:                  return EPixelFormat::D32Float;
    case VK_FORMAT_D32_SFLOAT_S8_UINT:          return EPixelFormat::D32FloatS8X24UInt;

    case VK_FORMAT_S8_UINT:                     return EPixelFormat::S8UInt;

    }

    ConversionFailed("VkFormat", "EPixelFormat");

    return EPixelFormat::Undefined;
}

VkCommandBufferLevel VulkanTypeConverter::ConvertCmdBuffFlagsToVk(uint32 inFlags)
{
    if (inFlags & CommandBufferLevelFlags::Primary)
    {
        return VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    }

    if (inFlags & CommandBufferLevelFlags::Secondary)
    {
        return VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    }

    ConversionFailed("CommandBufferFlags", "VkCommandBufferLevel");

    return VkCommandBufferLevel();
}

VkBufferUsageFlags VulkanTypeConverter::ConvertBufferUsageFlagsToVk(EBufferUsageFlags inUsageFlag)
{
    switch (inUsageFlag)
    {
    case EBufferUsageFlags::Vertex:
        return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        break;
    case EBufferUsageFlags::Index:
        return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        break;
    case EBufferUsageFlags::Storage:
        return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        break;
    }

    ConversionFailed("EBufferUsageFlags", "VkBufferUsageFlags");

    return (VkBufferUsageFlags)0;
}

VkMemoryPropertyFlags VulkanTypeConverter::ConvertMemoryFlagsToVk(uint32 inMemoryFlags)
{
    VkMemoryPropertyFlags Flags = (VkMemoryPropertyFlags)0;

    if (inMemoryFlags & MemoryFlags::DeviceLocal)
    {
        Flags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    }

    if (inMemoryFlags & MemoryFlags::HostCached)
    {
        Flags |= VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
    }

    if (inMemoryFlags & MemoryFlags::HostCoherent)
    {
        Flags |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    }

    if (inMemoryFlags & MemoryFlags::HostVisible)
    {
        Flags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    }

    return Flags;
}

VkSampleCountFlagBits VulkanTypeConverter::ConvertSampleCountToVk(uint32 inSamples)
{
    switch (inSamples)
    {
    case 1:     return VK_SAMPLE_COUNT_1_BIT;
    case 2:     return VK_SAMPLE_COUNT_2_BIT;
    case 4:     return VK_SAMPLE_COUNT_4_BIT;
    case 8:     return VK_SAMPLE_COUNT_8_BIT;
    case 16:    return VK_SAMPLE_COUNT_16_BIT;
    case 32:    return VK_SAMPLE_COUNT_32_BIT;
    case 64:    return VK_SAMPLE_COUNT_64_BIT;
    default:
        break;
    }

    ConversionFailed("VkSampleCountFlagBits", "uint32(sample count)");

    return (VkSampleCountFlagBits)0;
}

VkImageLayout VulkanTypeConverter::ConvertTextureLayoutToVk(ETextureLayout inLayout)
{
    switch (inLayout)
    {
    case ETextureLayout::Undefined:                  return VK_IMAGE_LAYOUT_UNDEFINED;
    case ETextureLayout::ColorAttachment:            return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    case ETextureLayout::DepthStencilAttachment:     return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    case ETextureLayout::DepthStencilReadOnly:       return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    case ETextureLayout::PresentSrc:                 return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    default:
        break;
    }

    ConversionFailed("VkImageLayout", "ETextureLayout");

    return (VkImageLayout)0;
}

VkAttachmentDescription VulkanTypeConverter::ConvertAttachmentDescToVk(const AttachmentDescription& inDesc, VkSampleCountFlagBits inSamples)
{
    VkAttachmentDescription Desc;
    Desc.flags = 0;
    Desc.format = Convert(inDesc.Format);
    Desc.samples = inSamples;
    Desc.loadOp = ConvertAttachmentLoadOpToVk(inDesc.LoadOp);
    Desc.storeOp = ConvertAttachmentStoreOpToVk(inDesc.StoreOp);
    Desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    Desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    //Desc.initialLayout = (inDesc.LoadOp == EAttachmentLoadOp::Load ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_UNDEFINED);
   // Desc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    Desc.initialLayout = ConvertTextureLayoutToVk(inDesc.InitialLayout);
    Desc.finalLayout = ConvertTextureLayoutToVk(inDesc.FinalLayout);

    return Desc;
}

VkAttachmentLoadOp VulkanTypeConverter::ConvertAttachmentLoadOpToVk(const EAttachmentLoadOp inLoadOp)
{
    switch (inLoadOp)
    {
    case EAttachmentLoadOp::Undefined:   return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    case EAttachmentLoadOp::Load:        return VK_ATTACHMENT_LOAD_OP_LOAD;
    case EAttachmentLoadOp::Clear:       return VK_ATTACHMENT_LOAD_OP_CLEAR;
    }

    ConversionFailed("VkAttachmentLoadOp", "EAttachmentLoadOp");

    return (VkAttachmentLoadOp)0;
}

VkAttachmentStoreOp VulkanTypeConverter::ConvertAttachmentStoreOpToVk(const EAttachmentStoreOp inStoreOp)
{
    switch (inStoreOp)
    {
    case EAttachmentStoreOp::Undefined:  return VK_ATTACHMENT_STORE_OP_DONT_CARE;
    case EAttachmentStoreOp::Store:      return VK_ATTACHMENT_STORE_OP_STORE;
    }

    ConversionFailed("VkAttachmentStoreOp", "EAttachmentStoreOp");

    return (VkAttachmentStoreOp)0;
}

VkDescriptorType VulkanTypeConverter::ConvertPipelineBDToVk(const PipelineBindingDescriptor inDesc)
{
    switch (inDesc.ResourceType)
    {
    case EResourceType::Buffer:
        if (inDesc.BindFlags & ResourceBindFlags::ConstantBuffer)
        {
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        }

        if (inDesc.BindFlags & (ResourceBindFlags::StorageBuffer))
        {
            return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        }

        break;
    case EResourceType::Texture:
        return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    case EResourceType::Sampler:
        return VK_DESCRIPTOR_TYPE_SAMPLER;
    default:
        break;
    }

    ConversionFailed("VkDescriptorType", "PipelineBindingDescriptor");

    return (VkDescriptorType)0;
}

VkShaderStageFlags VulkanTypeConverter::ConvertShaderFlagsToVk(uint32 inFlags)
{
    VkShaderStageFlags Flags = 0;

    if (inFlags & ShaderStageFlags::VertexStage)            { Flags |= VK_SHADER_STAGE_VERTEX_BIT; }
    if (inFlags & ShaderStageFlags::TessControlStage)       { Flags |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT; }
    if (inFlags & ShaderStageFlags::TessEvaluationStage)    { Flags |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT; }
    if (inFlags & ShaderStageFlags::GeometryStage)          { Flags |= VK_SHADER_STAGE_GEOMETRY_BIT; }
    if (inFlags & ShaderStageFlags::FragmentStage)          { Flags |= VK_SHADER_STAGE_FRAGMENT_BIT; }
    if (inFlags & ShaderStageFlags::ComputeStage)           { Flags |= VK_SHADER_STAGE_COMPUTE_BIT; }

    return Flags;
}

VkPrimitiveTopology VulkanTypeConverter::ConvertTopologyToVk(EPrimitiveTopology inTopology)
{
    switch (inTopology)
    {
    case EPrimitiveTopology::PointList:                return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    case EPrimitiveTopology::LineList:                 return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    case EPrimitiveTopology::LineStrip:                return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
    case EPrimitiveTopology::LineListAdjacency:        return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
    case EPrimitiveTopology::LineStripAdjacency:       return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
    case EPrimitiveTopology::TriangleList:             return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    case EPrimitiveTopology::TriangleStrip:            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    case EPrimitiveTopology::TriangleListAdjacency:    return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
    case EPrimitiveTopology::TriangleStripAdjacency:   return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
    default:
        break;
    }

    ConversionFailed("VkPrimitiveTopology", "EPrimitiveTopology");

    return (VkPrimitiveTopology)0;
}

VkViewport VulkanTypeConverter::ConvertViewportToVk(const RenderViewport& inViewport)
{
    VkViewport Viewport;
    Viewport.x = inViewport.X;
    Viewport.y = inViewport.Y;

    Viewport.width = inViewport.Width;
    Viewport.height = inViewport.Height;

    Viewport.minDepth = inViewport.MinDepth;
    Viewport.maxDepth = inViewport.MaxDepth;

    return Viewport;
}

VkRect2D VulkanTypeConverter::ConvertScissorToVk(const RenderScissor& inViewport)
{
    VkRect2D Rect;

    Rect.extent.width = inViewport.Width;
    Rect.extent.height = inViewport.Height;

    Rect.offset.x = inViewport.OffsetX;
    Rect.offset.y = inViewport.OffsetY;

    return Rect;
}

VkPolygonMode VulkanTypeConverter::ConvertPolygonModeToVk(EPolygonMode inMode)
{
    switch (inMode)
    {
    case EPolygonMode::Fill:    return VK_POLYGON_MODE_FILL;
    case EPolygonMode::Line:    return VK_POLYGON_MODE_LINE;
    case EPolygonMode::Point:   return VK_POLYGON_MODE_POINT;
    default:
        break;
    }

    ConversionFailed("VkPolygonMode", "EPolygonMode");

    return (VkPolygonMode)0;
}

VkCullModeFlags VulkanTypeConverter::ConvertCullModeToVk(ECullMode inMode)
{
    VkCullModeFlags Flags = 0;
    switch (inMode)
    {
    case ECullMode::Front:
        Flags |= VK_CULL_MODE_FRONT_BIT;
        break;
    case ECullMode::Back:
        Flags |= VK_CULL_MODE_BACK_BIT;
        break;
    default:
        break;
    }

    return Flags;
}

VkFrontFace VulkanTypeConverter::ConvertFrontFaceToVk(EFrontFace inFace)
{
    switch (inFace)
    {
    case EFrontFace::CounterClockwise:  return VK_FRONT_FACE_COUNTER_CLOCKWISE;
    case EFrontFace::Clockwise:         return VK_FRONT_FACE_CLOCKWISE;
    default:
        break;
    }

    ConversionFailed("VkFrontFace", "EFrontFace");

    return (VkFrontFace)0;
}

VkStencilOp VulkanTypeConverter::ConvertStencilOpToVk(EStencilOp inOp)
{
    switch (inOp)
    {
    case EStencilOp::Keep:                      return VK_STENCIL_OP_KEEP;
    case EStencilOp::Zero:                      return VK_STENCIL_OP_ZERO;
    case EStencilOp::Replace:                   return VK_STENCIL_OP_REPLACE;
    case EStencilOp::IncrementAndClamp:         return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
    case EStencilOp::DecrementAndClamp:         return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
    case EStencilOp::Invert:                    return VK_STENCIL_OP_INVERT;
    case EStencilOp::IncrementAndWrap:          return VK_STENCIL_OP_INCREMENT_AND_WRAP;
    case EStencilOp::DecrementAndWrap:          return VK_STENCIL_OP_DECREMENT_AND_WRAP;
    default:
        break;
    }

    ConversionFailed("VkStencilOp", "EStencilOp");

    return (VkStencilOp)0;
}

VkCompareOp VulkanTypeConverter::ConvertCompareOpToVk(ECompareOp inOp)
{
    switch (inOp)
    {
    case ECompareOp::Never:             return VK_COMPARE_OP_NEVER;
    case ECompareOp::Less:              return VK_COMPARE_OP_LESS;
    case ECompareOp::Equal:             return VK_COMPARE_OP_EQUAL;
    case ECompareOp::LessOrEqual:       return VK_COMPARE_OP_LESS_OR_EQUAL;
    case ECompareOp::Greater:           return VK_COMPARE_OP_GREATER;
    case ECompareOp::NotEqual:          return VK_COMPARE_OP_NOT_EQUAL;
    case ECompareOp::GreaterOrEqual:    return VK_COMPARE_OP_GREATER_OR_EQUAL;
    case ECompareOp::Always:            return VK_COMPARE_OP_ALWAYS;
    default:
        break;
    }

    ConversionFailed("VkCompareOp", "ECompareOp");

    return (VkCompareOp)0;
}

VkBlendFactor VulkanTypeConverter::ConvertBlendFactorToVk(EBlendFactor inFactor)
{
    switch (inFactor)
    {
    case EBlendFactor::Zero:                        return VK_BLEND_FACTOR_ZERO;
    case EBlendFactor::One:                         return VK_BLEND_FACTOR_ONE;
    case EBlendFactor::SrcColor:                    return VK_BLEND_FACTOR_SRC_COLOR;
    case EBlendFactor::OneMinusSrcColor:            return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
    case EBlendFactor::DstColor:                    return VK_BLEND_FACTOR_DST_COLOR;
    case EBlendFactor::OneMinusDstColor:            return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
    case EBlendFactor::SrcAlpha:                    return VK_BLEND_FACTOR_SRC_ALPHA;
    case EBlendFactor::OneMinusSrcAlpha:            return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    case EBlendFactor::DstAlpha:                    return VK_BLEND_FACTOR_DST_ALPHA;
    case EBlendFactor::OneMinusDstAlpha:            return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
    case EBlendFactor::ConstantColor:               return VK_BLEND_FACTOR_CONSTANT_COLOR;
    case EBlendFactor::OneMinusConstantColor:       return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
    case EBlendFactor::ConstantAlpha:               return VK_BLEND_FACTOR_CONSTANT_ALPHA;
    case EBlendFactor::OneMinusConstantAlpha:       return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
    case EBlendFactor::SrcAlphaSaturate:            return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
    case EBlendFactor::Src1Color:                   return VK_BLEND_FACTOR_SRC1_COLOR;
    case EBlendFactor::OneMinusSrc1Color:           return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
    case EBlendFactor::Src1Alpha:                   return VK_BLEND_FACTOR_SRC1_ALPHA;
    case EBlendFactor::OneMinusSrc1Alpha:           return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
    default:
        break;
    }

    ConversionFailed("VkBlendFactor", "EBlendFactor");

    return (VkBlendFactor)0;
}

VkBlendOp VulkanTypeConverter::ConvertBlendOpToVk(EBlendOp inOp)
{
    switch (inOp)
    {
    case EBlendOp::Add:                 return VK_BLEND_OP_ADD;
    case EBlendOp::Subtract:            return VK_BLEND_OP_SUBTRACT;
    case EBlendOp::ReverseSubtract:     return VK_BLEND_OP_REVERSE_SUBTRACT;
    case EBlendOp::Min:                 return VK_BLEND_OP_MIN;
    case EBlendOp::Max:                 return VK_BLEND_OP_MAX;
    default:
        break;
    }

    ConversionFailed("VkBlendOp", "EBlendOp");

    return (VkBlendOp)0;
}

VkColorComponentFlags VulkanTypeConverter::ConvertColorComponentMaskToVk(uint8 inColorMask)
{
    VkColorComponentFlags Flags = 0;

    if (inColorMask & ColorComponentFlags::R) { Flags |= VK_COLOR_COMPONENT_R_BIT; }
    if (inColorMask & ColorComponentFlags::G) { Flags |= VK_COLOR_COMPONENT_G_BIT; }
    if (inColorMask & ColorComponentFlags::B) { Flags |= VK_COLOR_COMPONENT_B_BIT; }
    if (inColorMask & ColorComponentFlags::A) { Flags |= VK_COLOR_COMPONENT_A_BIT; }

    return Flags;
}

VkLogicOp VulkanTypeConverter::ConvertLogicOpToVk(ELogicOp inOp)
{
    switch (inOp)
    {
    case ELogicOp::Disabled: break;
    case ELogicOp::Clear:               return VK_LOGIC_OP_CLEAR;
    case ELogicOp::And:                 return VK_LOGIC_OP_AND;
    case ELogicOp::AndReverse:          return VK_LOGIC_OP_AND_REVERSE;
    case ELogicOp::Copy:                return VK_LOGIC_OP_COPY;
    case ELogicOp::AndInverted:         return VK_LOGIC_OP_AND_INVERTED;
    case ELogicOp::NoOp:                return VK_LOGIC_OP_NO_OP;
    case ELogicOp::XOR:                 return VK_LOGIC_OP_XOR;
    case ELogicOp::Or:                  return VK_LOGIC_OP_OR;
    case ELogicOp::Nor:                 return VK_LOGIC_OP_NOR;
    case ELogicOp::Equivalent:          return VK_LOGIC_OP_EQUIVALENT;
    case ELogicOp::Invert:              return VK_LOGIC_OP_INVERT;
    case ELogicOp::OrReverse:           return VK_LOGIC_OP_OR_REVERSE;
    case ELogicOp::CopyInverted:        return VK_LOGIC_OP_COPY_INVERTED;
    case ELogicOp::OrInverted:          return VK_LOGIC_OP_OR_INVERTED;
    case ELogicOp::NAND:                return VK_LOGIC_OP_NAND;
    case ELogicOp::Set:                 return VK_LOGIC_OP_SET;
    default:
        break;
    }

    ConversionFailed("VkLogicOp", "ELogicOp");

    return (VkLogicOp)0;
}

VkShaderStageFlagBits VulkanTypeConverter::ConvertShaderTypeToVk(EShaderType inType)
{
    switch (inType)
    {
    case EShaderType::Vertex:               return VK_SHADER_STAGE_VERTEX_BIT;
    case EShaderType::TessControl:          return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    case EShaderType::TessEvaluation:       return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    case EShaderType::Geometry:             return VK_SHADER_STAGE_GEOMETRY_BIT;
    case EShaderType::Fragment:             return VK_SHADER_STAGE_FRAGMENT_BIT;
    case EShaderType::Compute:              return VK_SHADER_STAGE_COMPUTE_BIT;
    default:
        break;
    }

    ConversionFailed("VkShaderStageFlagBits", "EShaderType");

    return (VkShaderStageFlagBits)0;
}

VkImageType VulkanTypeConverter::ConvertTextureTypeToVk(ETextureType inType)
{
    switch (inType)
    {
    case ETextureType::Texture1D: return VK_IMAGE_TYPE_1D;
    case ETextureType::Texture2D: return VK_IMAGE_TYPE_2D;
    case ETextureType::Texture3D: return VK_IMAGE_TYPE_3D;
    default:
        break;
    }

    ConversionFailed("VkImageType", "ETextureType");

    return (VkImageType)0;
}

VkImageUsageFlags VulkanTypeConverter::ConvertTextureUsageFlagsToVk(uint32 inFlags)
{
    VkImageUsageFlags Flags = 0;

    if (inFlags & ResourceBindFlags::ColorAttachment)
    {
        Flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }

    if (inFlags & ResourceBindFlags::DepthStencilAttachment)
    {
        Flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }

    if (inFlags & ResourceBindFlags::StorageBuffer)
    {
        Flags |= VK_IMAGE_USAGE_STORAGE_BIT;
    }

    return Flags;
}

VkImageViewType VulkanTypeConverter::ConvertTextureViewTypeToVk(ETextureType inType)
{
    switch (inType)
    {
    case ETextureType::Texture1D:           return VK_IMAGE_VIEW_TYPE_1D;
    case ETextureType::Texture2D:           return VK_IMAGE_VIEW_TYPE_2D;
    case ETextureType::Texture3D:           return VK_IMAGE_VIEW_TYPE_3D;
    case ETextureType::TextureCube:         return VK_IMAGE_VIEW_TYPE_CUBE;
    case ETextureType::Texture1DArray:      return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
    case ETextureType::Texture2DArray:      return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    case ETextureType::TextureCubeArray:    return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
    default:
        break;
    }

    ConversionFailed("VkImageViewType", "ETextureType");

    return (VkImageViewType)0;
}
