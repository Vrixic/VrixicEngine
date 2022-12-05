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

VulkanVertexShader* VulkanShaderFactory::CreateVertexShader(VulkanDevice* inDevice, const char* inShaderPath)
{
	VulkanVertexShader* VertexShader = new VulkanVertexShader(inDevice);
	VertexShader->ShaderKey = ResourceManagerHandle->CreateShaderResource(inShaderPath);

	return VertexShader;
}
