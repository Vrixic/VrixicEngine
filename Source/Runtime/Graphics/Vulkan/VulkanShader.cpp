#include "VulkanDevice.h"
#include "VulkanShader.h"

/* ------------------------------------------------------------------------------- */
/* -----------------------          VulkanShader         ------------------------- */
/* ------------------------------------------------------------------------------- */

VulkanShader::VulkanShader(VulkanDevice* inDevice, VkShaderStageFlagBits inShaderStageBits)
{
	ShaderKey = UINT32_MAX;
	Device = inDevice;
	ShaderStageBits = inShaderStageBits;
}

VulkanShader::~VulkanShader() { }

/* ------------------------------------------------------------------------------- */
/* -----------------------         TVulkanShader         ------------------------- */
/* ------------------------------------------------------------------------------- */

template<typename VkShaderStageFlagBits ShaderStageFlagBits>
TVulkanShader<ShaderStageFlagBits>::TVulkanShader(VulkanDevice* inDevice)
	: VulkanShader(inDevice, ShaderStageBits) { }

/* ------------------------------------------------------------------------------- */
/* -----------------------      VulkanShaderFactory      ------------------------- */
/* ------------------------------------------------------------------------------- */

VulkanShaderFactory::VulkanShaderFactory(ResourceManager* inResourceManagerHandle)
{
	ResourceManagerHandle = inResourceManagerHandle;
}

VulkanShaderFactory::~VulkanShaderFactory() { }

VulkanVertexShader* VulkanShaderFactory::CreateVertexShaderFromPath(VulkanDevice* inDevice, const char* inShaderPath)
{
	VulkanVertexShader* VertexShader = new VulkanVertexShader(inDevice);
	VertexShader->ShaderKey = ResourceManagerHandle->CreateShaderResourceFromPath(inShaderPath, (uint32)EShaderType::Vertex);

	return VertexShader;
}

VulkanVertexShader* VulkanShaderFactory::CreateVertexShaderFromString(VulkanDevice* inDevice, const char* inShaderStr)
{
	VulkanVertexShader* VertexShader = new VulkanVertexShader(inDevice);
	VertexShader->ShaderKey = ResourceManagerHandle->CreateShaderResourceFromString(inShaderStr, (uint32)EShaderType::Vertex);

	return VertexShader;
}

VulkanFragmentShader* VulkanShaderFactory::CreateFragmentShaderFromString(VulkanDevice* inDevice, const char* inShaderStr)
{
	VulkanFragmentShader* VertexShader = new VulkanFragmentShader(inDevice);
	VertexShader->ShaderKey = ResourceManagerHandle->CreateShaderResourceFromString(inShaderStr, (uint32)EShaderType::Fragment);

	return VertexShader;
}
