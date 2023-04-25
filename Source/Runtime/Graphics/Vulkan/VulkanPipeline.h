/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#pragma once
#include "VulkanDescriptorSet.h"
#include <Runtime/Graphics/Pipeline.h>
#include <Runtime/Graphics/PipelineLayout.h>
#include <Runtime/Graphics/Vulkan/VulkanShader.h>
#include "VulkanRenderPass.h"

/* ------------------------------------------------------------------------------- */
/**
* @TODO: Complete vulkan pipeline creation
*/
/* ------------------------------------------------------------------------------- */

/**
* Defines pipeline creation information, also includes a descriptor set layouts and push constant ranges
*/
class VRIXIC_API VulkanPipelineLayout : public PipelineLayout
{
public:
    VulkanPipelineLayout(VulkanDevice* inDevice, const FPipelineLayoutConfig& inPipelineLayoutConfig)
        : PipelineLayout(inPipelineLayoutConfig), Device(inDevice), PipelineLayoutHandle(VK_NULL_HANDLE)
    {
        DescriptorSetsLayout = new VulkanDescriptorSetsLayout(Device);

        VulkanUtils::Descriptions::FDescriptorSetLayoutCreateInfo LayoutCreateInfo = { };
        LayoutCreateInfo.Flags = 0;

        VulkanUtils::Descriptions::FDescriptorSetLayoutBinding* LayoutBinding = new VulkanUtils::Descriptions::FDescriptorSetLayoutBinding[inPipelineLayoutConfig.Bindings.size()];
        for (uint32 i = 0; i < inPipelineLayoutConfig.Bindings.size(); i++)
        {
            Convert(LayoutBinding[i], inPipelineLayoutConfig.Bindings[i]);
        }

        DescriptorSetLayoutIndex = DescriptorSetsLayout->CreateDescriptorSetLayout(LayoutBinding, inPipelineLayoutConfig.Bindings.size(), LayoutCreateInfo);

        LayoutBindings.resize(inPipelineLayoutConfig.Bindings.size());
        for (uint32 i = 0; i < inPipelineLayoutConfig.Bindings.size(); i++)
        {
            LayoutBindings[i].DstBinding = inPipelineLayoutConfig.Bindings[i].BindingSlot.Index;
            LayoutBindings[i].DescriptorType = (VkDescriptorType)LayoutBinding[i].DescriptorType;
            LayoutBindings[i].StageFlags = inPipelineLayoutConfig.Bindings[i].StageFlags;;
        }

        // DescriptorPool
        std::vector<VkDescriptorPoolSize> PoolSizes;
        uint32 MaxSets = 0;

        uint32 CountPerType[11] = { 0,0,0,0,0,0,0,0,0,0,0 };
        for (uint32 i = 0; i < inPipelineLayoutConfig.Bindings.size(); i++)
        {
            CountPerType[LayoutBinding[i].DescriptorType] += LayoutBinding[i].DescriptorCount;
            MaxSets += LayoutBinding[i].DescriptorCount;
        }

        for (uint32 i = 0; i < 11; i++)
        {
            if (CountPerType[i] > 0)
            {
                VkDescriptorPoolSize PoolSize = { };
                PoolSize.type = (VkDescriptorType)i;
                PoolSize.descriptorCount = CountPerType[i];

                PoolSizes.push_back(PoolSize);
            }
        }

        delete[] LayoutBinding;
        //delete[] CountPerType;
        DescriptorPool = new VulkanDescriptorPool(Device, *DescriptorSetsLayout, MaxSets > 0 ? MaxSets : 2, PoolSizes);
    }

    ~VulkanPipelineLayout()
    {
        Device->WaitUntilIdle();

        delete DescriptorSetsLayout;
        delete DescriptorPool;
        vkDestroyPipelineLayout(*Device->GetDeviceHandle(), PipelineLayoutHandle, nullptr);
    }

    VulkanPipelineLayout(const VulkanPipelineLayout& other) = delete;
    VulkanPipelineLayout operator=(const VulkanPipelineLayout& other) = delete;

public:
    /*
    * Should Only be called once
    */
    void Create(std::vector<VkPushConstantRange>* inPushConstants)
    {
        VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo = VulkanUtils::Initializers::PipelineLayoutCreateInfo();
        PipelineLayoutCreateInfo.pSetLayouts = DescriptorSetsLayout->DescriptorSetLayoutHandles.data();
        PipelineLayoutCreateInfo.setLayoutCount = DescriptorSetsLayout->DescriptorSetLayoutHandles.size();

        if (inPushConstants != nullptr)
        {
            PipelineLayoutCreateInfo.pPushConstantRanges = inPushConstants->data();
            PipelineLayoutCreateInfo.pushConstantRangeCount = inPushConstants->size();
        }

        VE_ASSERT(PipelineLayoutHandle == VK_NULL_HANDLE, "[VulkanPipelineLayout]: Failed to create another pipeline layout! Can only be created ONCE!");

        VK_CHECK_RESULT(vkCreatePipelineLayout(*Device->GetDeviceHandle(), &PipelineLayoutCreateInfo, nullptr,
            &PipelineLayoutHandle), "[VulkanPipelineLayout]: Failed to create a pipeline layout!");
    }

    /*
    * Should Only be called once -- DEPRECATED
    */
    void Create(VulkanDescriptorSetsLayout* inDescriptorSetsLayout, std::vector<VkPushConstantRange>* inPushConstants)
    {
        VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo = VulkanUtils::Initializers::PipelineLayoutCreateInfo();
        PipelineLayoutCreateInfo.pSetLayouts = inDescriptorSetsLayout->DescriptorSetLayoutHandles.data();
        PipelineLayoutCreateInfo.setLayoutCount = inDescriptorSetsLayout->DescriptorSetLayoutHandles.size();

        if (inPushConstants != nullptr)
        {
            PipelineLayoutCreateInfo.pPushConstantRanges = inPushConstants->data();
            PipelineLayoutCreateInfo.pushConstantRangeCount = inPushConstants->size();
        }

        VE_ASSERT(PipelineLayoutHandle == VK_NULL_HANDLE, "[VulkanPipelineLayout]: Failed to create another pipeline layout! Can only be created ONCE!");

        VK_CHECK_RESULT(vkCreatePipelineLayout(*Device->GetDeviceHandle(), &PipelineLayoutCreateInfo, nullptr,
            &PipelineLayoutHandle), "[VulkanPipelineLayout]: Failed to create a pipeline layout!");
    }

    void CreateEmpty()
    {
        VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo = VulkanUtils::Initializers::PipelineLayoutCreateInfo();

        VE_ASSERT(PipelineLayoutHandle == VK_NULL_HANDLE, "[VulkanPipelineLayout]: Failed to create another empty pipeline layout! Can only be created ONCE!");

        VK_CHECK_RESULT(vkCreatePipelineLayout(*Device->GetDeviceHandle(), &PipelineLayoutCreateInfo, nullptr,
            &PipelineLayoutHandle), "[VulkanPipelineLayout]: Failed to create a empty pipeline layout!");
    }

    // helpers
    static void Convert(VulkanUtils::Descriptions::FDescriptorSetLayoutBinding& outDst, const FPipelineBindingDescriptor& inSrc)
    {
        outDst.Binding = inSrc.BindingSlot.Index;
        outDst.DescriptorType = VulkanTypeConverter::ConvertPipelineBDToVk(inSrc);
        outDst.DescriptorCount = MathUtils::Max(1u, inSrc.NumResources);
        outDst.StageFlags = VulkanTypeConverter::ConvertShaderFlagsToVk(inSrc.StageFlags);
    }

public:
    inline const VkPipelineLayout* GetPipelineLayoutHandle() const
    {
        return &PipelineLayoutHandle;
    }

    inline const VulkanDescriptorPool* GetDescriptorPool() const
    {
        return DescriptorPool;
    }

private:
    VulkanDevice* Device;
    VkPipelineLayout PipelineLayoutHandle;

    /** Vulkan Descriptor pool - main pool used for all descriptor set creations */
    VulkanDescriptorPool* DescriptorPool;

    /** Layout of descriptor sets */
    VulkanDescriptorSetsLayout* DescriptorSetsLayout;
    uint32 DescriptorSetLayoutIndex;

    struct VRIXIC_API VulkanLayoutBinding
    {
        uint32 DstBinding;
        uint32 StageFlags;
        VkDescriptorType DescriptorType;
    };
    /** all of the pipeline layout bindings */ 
    std::vector<VulkanLayoutBinding> LayoutBindings;
};

/**
* Wrapper for vulkan pipeline
*	Do not create this object, use VulkanGraphicsPipeline..
*/
class VRIXIC_API VulkanPipeline : public IPipeline
{
public:
    ~VulkanPipeline()
    {
        Device->WaitUntilIdle();
        if (PipelineHandle != VK_NULL_HANDLE)
        {
            vkDestroyPipeline(*Device->GetDeviceHandle(), PipelineHandle, nullptr);
        }
    }

    VulkanPipeline(const VulkanPipeline& other) = delete;
    VulkanPipeline operator=(const VulkanPipeline& other) = delete;

public:
    inline virtual EPipelineBindPoint GetBindPoint() const override
    {
        return EPipelineBindPoint::Undefined;
    }

    inline const VkPipeline* GetPipelineHandle() const
    {
        return &PipelineHandle;
    }

    inline const VulkanPipelineLayout* GetPipelineLayout() const
    {
        return PipelineLayoutPtr;
    }

protected:
    VulkanDevice* Device;
    VkPipeline PipelineHandle;

    VulkanPipelineLayout* PipelineLayoutPtr;

protected:
    VulkanPipeline(VulkanDevice* inDevice)
        : Device(inDevice), PipelineLayoutPtr(VK_NULL_HANDLE), PipelineHandle(VK_NULL_HANDLE) {}
};

/**
* Represents a graphics vulkan pipeline
*/
class VRIXIC_API VulkanGraphicsPipeline : public VulkanPipeline
{
public:
    VulkanGraphicsPipeline(VulkanDevice* inDevice)
        : VulkanPipeline(inDevice) {  }

public:
    /**
    * Creates a graphics pipeline
    *
    * @param inCreateInfo - graphics pipeline create info which is used for pipeline creation
    */
    void Create(VkGraphicsPipelineCreateInfo& inCreateInfo)
    {
        vkCreateGraphicsPipelines(*Device->GetDeviceHandle(), VK_NULL_HANDLE, 1, &inCreateInfo, nullptr, &PipelineHandle);
    }

    void Create(const FGraphicsPipelineConfig& inConfig)
    {
        // Only allow this to be called once
        VE_ASSERT(PipelineHandle == VK_NULL_HANDLE, VE_TEXT("[VulkanGraphicsPipeline]: cannot create another pipeline when one already exists!!!"));

        // set pipeline layout
        PipelineLayoutPtr = (VulkanPipelineLayout*)inConfig.PipelineLayoutPtr;

        // Check for shader status, a pipeline cannot be created without a vertex shader 
        VulkanShader* VertexShader = (VulkanShader*)inConfig.VertexShader;
        VE_ASSERT(VertexShader->GetShaderType() == EShaderType::Vertex, VE_TEXT("[VulkanGraphicsPipeline]: Cannot create a graphics pipeline without a vertex shader!!"));

        VkPipelineShaderStageCreateInfo ShaderStageCreateInfos[2];
        ShaderStageCreateInfos[0] = VulkanUtils::Initializers::PipelineShaderStageCreateInfo();
        ShaderStageCreateInfos[0].stage = VulkanTypeConverter::ConvertShaderTypeToVk(EShaderType::Vertex);
        ShaderStageCreateInfos[0].pName = "main";
        ShaderStageCreateInfos[0].module = VertexShader->GetShaderModuleHandle();

        VulkanShader* FragmentShader = (VulkanShader*)inConfig.FragmentShader;
        ShaderStageCreateInfos[1] = VulkanUtils::Initializers::PipelineShaderStageCreateInfo();
        ShaderStageCreateInfos[1].stage = VulkanTypeConverter::ConvertShaderTypeToVk(EShaderType::Fragment);
        ShaderStageCreateInfos[1].pName = "main";
        ShaderStageCreateInfos[1].module = FragmentShader->GetShaderModuleHandle();

        // Input assembly and vertex input state creation
        VkPipelineVertexInputStateCreateInfo VertexInputStateCreateInfo = VulkanUtils::Initializers::PipelineVertexInputStateCreateInfo();
        VertexShader->CreateVertexInputStateCreateInfo(VertexInputStateCreateInfo);

        VkPipelineInputAssemblyStateCreateInfo InputAssemblyStateCreateInfo = VulkanUtils::Initializers::PipelineInputAssemblyStateCreateInfo();
        CreateInputAssemblyState(inConfig, InputAssemblyStateCreateInfo);

        // Viewports and scissors
        std::vector<VkViewport> Viewports;
        std::vector<VkRect2D> Scissors;
        VkPipelineViewportStateCreateInfo ViewportStateCreateInfo = VulkanUtils::Initializers::PipelineViewportStateCreateInfo();
        CreateViewportState(inConfig, Viewports, Scissors, ViewportStateCreateInfo);

        // Rasterizer state
        VkPipelineRasterizationStateCreateInfo RasterizationStateCreateInfo;
        CreateRasterizerState(inConfig.RasterizerState, RasterizationStateCreateInfo);

        // Multisampling 
        VkPipelineMultisampleStateCreateInfo MultisampleStateCreateInfo = VulkanUtils::Initializers::PipelineMultisampleStateCreateInfo();
        VulkanRenderPass* RenderPass = (VulkanRenderPass*)inConfig.RenderPassPtr;
        CreateMultisampleState(RenderPass->GetSampleCountFlagBits(), inConfig.BlendState, MultisampleStateCreateInfo);

        // depth stencil state
        VkPipelineDepthStencilStateCreateInfo DepthStencilStateCreateInfo;
        CreateDepthStencilState(inConfig, DepthStencilStateCreateInfo);

        // Color-blend state 
        std::vector<VkPipelineColorBlendAttachmentState> ColorBlendAttachmentStates;
        VkPipelineColorBlendStateCreateInfo ColorBlendStateCreateInfo;
        CreateColorBlendState(inConfig.BlendState, RenderPass->GetNumColorAttachments(), ColorBlendAttachmentStates, ColorBlendStateCreateInfo);

        // Dynamic state
        std::vector<VkDynamicState> DynamicStates;
        VkPipelineDynamicStateCreateInfo DynamicStateCreateInfo;
        CreateDynamicState(inConfig, DynamicStates, DynamicStateCreateInfo);

        VkGraphicsPipelineCreateInfo GraphicsPipelineCreateInfo = VulkanUtils::Initializers::GraphicsPipelineCreateInfo();
        GraphicsPipelineCreateInfo.stageCount = 2;
        GraphicsPipelineCreateInfo.pStages = ShaderStageCreateInfos;
        GraphicsPipelineCreateInfo.pVertexInputState = &VertexInputStateCreateInfo;
        GraphicsPipelineCreateInfo.pInputAssemblyState = &InputAssemblyStateCreateInfo;
        GraphicsPipelineCreateInfo.pViewportState = &ViewportStateCreateInfo;
        GraphicsPipelineCreateInfo.pRasterizationState = &RasterizationStateCreateInfo;
        GraphicsPipelineCreateInfo.pMultisampleState = &MultisampleStateCreateInfo;
        GraphicsPipelineCreateInfo.pDepthStencilState = &DepthStencilStateCreateInfo;
        GraphicsPipelineCreateInfo.pColorBlendState = &ColorBlendStateCreateInfo;
        GraphicsPipelineCreateInfo.pDynamicState = &DynamicStateCreateInfo;
        GraphicsPipelineCreateInfo.layout = *GetPipelineLayout()->GetPipelineLayoutHandle();
        GraphicsPipelineCreateInfo.renderPass = *RenderPass->GetRenderPassHandle();

        VK_CHECK_RESULT(vkCreateGraphicsPipelines(*Device->GetDeviceHandle(), VK_NULL_HANDLE, 1, &GraphicsPipelineCreateInfo, nullptr, &PipelineHandle), VE_TEXT("[VulkanGraphicsPipeline]: Failed to create a vulkan graphics pipeline!!"));
    }

public:
    inline virtual EPipelineBindPoint GetBindPoint() const override final
    {
        return EPipelineBindPoint::Graphics;
    }

private:
    /**
    * Creates an input assembly state from the graphics pipeline configuration passed in
    *
    * @param inConfig the pipeline configuration
    * @param outIACreateInfo gets filled with the input assembly information from 'inConfig'
    */
    static void CreateInputAssemblyState(const FGraphicsPipelineConfig& inConfig, VkPipelineInputAssemblyStateCreateInfo& outIACreateInfo)
    {
        outIACreateInfo = VulkanUtils::Initializers::PipelineInputAssemblyStateCreateInfo();
        outIACreateInfo.topology = VulkanTypeConverter::ConvertTopologyToVk(inConfig.PrimitiveTopology);
        outIACreateInfo.primitiveRestartEnable = false;
    }

    /**
    * Creates a viewport state from the graphics pipeline configuration passed in
    *
    * @param inConfig the pipeline configuration
    * @param outVSCreateInfo gets filled with the viewport state information from 'inConfig'
    */
    static void CreateViewportState(const FGraphicsPipelineConfig& inConfig, std::vector<VkViewport>& outViewports, std::vector<VkRect2D>& outScissors, VkPipelineViewportStateCreateInfo& outVSCreateInfo)
    {
        outVSCreateInfo = VulkanUtils::Initializers::PipelineViewportStateCreateInfo();

        const uint32 NumViewports = inConfig.Viewports.size();
        const uint32 NumScissors = inConfig.Scissors.size();

        // Default viewport values
        outVSCreateInfo.viewportCount = 1;
        outVSCreateInfo.pViewports = nullptr;
        if (NumViewports > 0)
        {
            outVSCreateInfo.viewportCount = NumViewports;

            outViewports.resize(NumViewports);

            for (uint32 i = 0; i < NumViewports; i++)
            {
                outViewports[i] = VulkanTypeConverter::ConvertViewportToVk(inConfig.Viewports[i]);
            }

            outVSCreateInfo.pViewports = outViewports.data();
        }

        // Default scissor values
        outVSCreateInfo.scissorCount = 1;
        outVSCreateInfo.pScissors = nullptr;
        if (NumScissors > 0)
        {
            outVSCreateInfo.scissorCount = NumScissors;

            outScissors.resize(NumScissors);

            for (uint32 i = 0; i < NumScissors; i++)
            {
                outScissors[i] = VulkanTypeConverter::ConvertScissorToVk(inConfig.Scissors[i]);
            }

            outVSCreateInfo.pScissors = outScissors.data();
        }
    }

    /**
    * Creates a rasterizer state from the rasterizer configuration passed in
    *
    * @param inConfig the rasterizer configuration
    * @param outRSCreateInfo gets filled with the rasterizer state information from 'inConfig'
    */
    static void CreateRasterizerState(const FRasterizerConfig& inConfig, VkPipelineRasterizationStateCreateInfo& outRSCreateInfo)
    {
        outRSCreateInfo = VulkanUtils::Initializers::PipelineRasterizationStateCreateInfo();

        outRSCreateInfo.depthBiasEnable = inConfig.bDepthClampEnabled;
        outRSCreateInfo.rasterizerDiscardEnable = inConfig.bRasterizerDiscardEnabled;
        outRSCreateInfo.polygonMode = VulkanTypeConverter::ConvertPolygonModeToVk(inConfig.PolygonMode);
        outRSCreateInfo.cullMode = VulkanTypeConverter::ConvertCullModeToVk(inConfig.CullMode);
        outRSCreateInfo.frontFace = VulkanTypeConverter::ConvertFrontFaceToVk(inConfig.FrontFace);
        outRSCreateInfo.depthBiasEnable = inConfig.bDepthBiasEnabled;
        outRSCreateInfo.depthBiasConstantFactor = inConfig.DepthBias.ConstantFactor;
        outRSCreateInfo.depthBiasClamp = inConfig.DepthBias.Clamp;
        outRSCreateInfo.depthBiasSlopeFactor = inConfig.DepthBias.SlopeFactor;
        // should check for pipeline limits -> if line width meets the limit conditions
        outRSCreateInfo.lineWidth = inConfig.LineWidth;
    }

    /**
    * Creates a multi sample state from the blend state configuration passed in
    *
    * @param inSampleCountBits count of sample for the rasterization samples taken
    * @param inBlendConfig the blend state configuration
    * @param outMSSCreateInfo gets filled with the multi sample state information from 'inBlendConfig'
    */
    static void CreateMultisampleState(const VkSampleCountFlagBits inSampleCountBits, const FBlendStateConfig& inBlendConfig, VkPipelineMultisampleStateCreateInfo& outMSSCreateInfo)
    {
        outMSSCreateInfo = VulkanUtils::Initializers::PipelineMultisampleStateCreateInfo();

        outMSSCreateInfo.rasterizationSamples = inSampleCountBits;
        outMSSCreateInfo.sampleShadingEnable = VK_FALSE;
        outMSSCreateInfo.minSampleShading = 0.0f;
        outMSSCreateInfo.pSampleMask = inBlendConfig.bAlphaToCoverageEnabled ? (const VkSampleMask*)&inBlendConfig.SampleMask : nullptr;
        outMSSCreateInfo.alphaToCoverageEnable = inBlendConfig.bAlphaToCoverageEnabled;
        outMSSCreateInfo.alphaToOneEnable = VK_FALSE;
    }

    /**
    * Creates a stencil op state from the stencil op state configuration passed in
    *
    * @param inConfig the stencil op state configuration
    * @param outSOCreateInfo gets filled with the stencil op state information from 'inConfig'
    */
    static void CreateStencilOpState(const FStencilOpConfig& inConfig, VkStencilOpState& outSOCreateInfo)
    {
        outSOCreateInfo = { };

        outSOCreateInfo.failOp = VulkanTypeConverter::ConvertStencilOpToVk(inConfig.StencilFailOp);
        outSOCreateInfo.passOp = VulkanTypeConverter::ConvertStencilOpToVk(inConfig.StencilPassOp);
        outSOCreateInfo.depthFailOp = VulkanTypeConverter::ConvertStencilOpToVk(inConfig.DepthFailOp);
        outSOCreateInfo.compareOp = VulkanTypeConverter::ConvertCompareOpToVk(inConfig.CompareOp);
        outSOCreateInfo.compareMask = inConfig.CompareMask;
        outSOCreateInfo.writeMask = inConfig.WriteMask;
        outSOCreateInfo.reference = inConfig.ReferenceValue;
    }

    /**
    * Creates a depth stencil state from the graphics pipeline configuration passed in
    *
    * @param inConfig the pipeline configuration
    * @param outSOCreateInfo gets filled with the depth stencil state information from 'inConfig'
    */
    static void CreateDepthStencilState(const FGraphicsPipelineConfig& inConfig, VkPipelineDepthStencilStateCreateInfo& outDPSCreateInfo)
    {
        outDPSCreateInfo = VulkanUtils::Initializers::PipelineDepthStencilStateCreateInfo();

        outDPSCreateInfo.depthTestEnable = inConfig.DepthState.bIsTestingEnabled;
        outDPSCreateInfo.depthWriteEnable = inConfig.DepthState.bIsWritingEnabled;
        outDPSCreateInfo.depthCompareOp = VulkanTypeConverter::ConvertCompareOpToVk(inConfig.DepthState.CompareOp);
        outDPSCreateInfo.depthBoundsTestEnable = VK_FALSE;
        outDPSCreateInfo.stencilTestEnable = inConfig.StencilState.bIsTestingEnabled;
        CreateStencilOpState(inConfig.StencilState.Front, outDPSCreateInfo.front);
        CreateStencilOpState(inConfig.StencilState.Back, outDPSCreateInfo.back);
        outDPSCreateInfo.minDepthBounds = 0.0f;
        outDPSCreateInfo.maxDepthBounds = 1.0f;
    }

    /**
    * Creates a color blend attachment state from the blend op configuration passed in
    *
    * @param inConfig the blend op configuration
    * @param outCBAState gets filled with the blend op information from 'inConfig'
    */
    static void CreateColorBlendAttachmentState(const FBlendOpConfig& inConfig, VkPipelineColorBlendAttachmentState& outCBAState)
    {
        outCBAState.blendEnable = inConfig.bIsBlendEnabled;
        outCBAState.srcColorBlendFactor = VulkanTypeConverter::ConvertBlendFactorToVk(inConfig.SrcColorBlendFactor);
        outCBAState.dstColorBlendFactor = VulkanTypeConverter::ConvertBlendFactorToVk(inConfig.DstColorBlendFactor);
        outCBAState.colorBlendOp = VulkanTypeConverter::ConvertBlendOpToVk(inConfig.ColorBlendOp);
        outCBAState.srcAlphaBlendFactor = VulkanTypeConverter::ConvertBlendFactorToVk(inConfig.SrcAlphaBlendFactor);
        outCBAState.dstAlphaBlendFactor = VulkanTypeConverter::ConvertBlendFactorToVk(inConfig.SrcAlphaBlendFactor);
        outCBAState.alphaBlendOp = VulkanTypeConverter::ConvertBlendOpToVk(inConfig.AlphaBlendOp);
        outCBAState.colorWriteMask = VulkanTypeConverter::ConvertColorComponentMaskToVk(inConfig.ColorWriteMask);
    }

    /**
    * Creates a color blend state from the blend state configuration passed in
    *
    * @param inConfig the blend op configuration
    * @param inNumColorAttachments number of blend ops/color attachments
    * @param outColorBlendAttachmentStates converted blend states that are specific to vulkan
    * @param outCBSCreateInfo gets filled with the blend state information from 'inConfig'
    */
    static void CreateColorBlendState(const FBlendStateConfig& inConfig, std::uint32_t inNumColorAttachments, std::vector<VkPipelineColorBlendAttachmentState>& outColorBlendAttachmentStates, VkPipelineColorBlendStateCreateInfo& outCBSCreateInfo)
    {
        outCBSCreateInfo = VulkanUtils::Initializers::PipelineColorBlendStateCreateInfo();

        // Default logic op values 
        outCBSCreateInfo.logicOpEnable = VK_FALSE;
        outCBSCreateInfo.logicOp = VK_LOGIC_OP_NO_OP;
        if (inConfig.LogicOp != ELogicOp::Disabled)
        {
            outCBSCreateInfo.logicOpEnable = VK_TRUE;
            outCBSCreateInfo.logicOp = VulkanTypeConverter::ConvertLogicOpToVk(inConfig.LogicOp);
        }

        // Convert blend ops to vulkan specific ones 
        outColorBlendAttachmentStates.resize(inNumColorAttachments);
        for (uint32 i = 0; i < inNumColorAttachments; i++)
        {
            uint32 Index = inConfig.bIndependentBlendEnabled ? i : 0;
            CreateColorBlendAttachmentState(inConfig.BlendOpConfigs[Index], outColorBlendAttachmentStates[i]);
        }

        outCBSCreateInfo.attachmentCount = inNumColorAttachments;
        outCBSCreateInfo.pAttachments = outColorBlendAttachmentStates.data();
        outCBSCreateInfo.blendConstants[0] = inConfig.BlendConstants[0];
        outCBSCreateInfo.blendConstants[1] = inConfig.BlendConstants[1];
        outCBSCreateInfo.blendConstants[2] = inConfig.BlendConstants[2];
        outCBSCreateInfo.blendConstants[3] = inConfig.BlendConstants[3];
    }

    /**
    * Creates a dynamic state from the graphics pipeline configuration passed in
    *
    * @param inConfig the pipeline configuration
    * @param outDynamicStates the converted dynamic states that are vulkan specific
    * @param outPDSCreateInfo gets filled with the dynamic state information from 'inConfig'
    */
    static void CreateDynamicState(const FGraphicsPipelineConfig& inConfig, std::vector<VkDynamicState>& outDynamicStates, VkPipelineDynamicStateCreateInfo& outPDSCreateInfo)
    {
        outPDSCreateInfo = VulkanUtils::Initializers::PipelineDynamicStateCreateInfo();

        if (inConfig.Viewports.empty())
        {
            outDynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
        }

        if (inConfig.Scissors.empty())
        {
            outDynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
        }

        if (inConfig.BlendState.bIsBlendFactorDynamic)
        {
            outDynamicStates.push_back(VK_DYNAMIC_STATE_BLEND_CONSTANTS);
        }

        if (inConfig.StencilState.bIsReferenceValueDynamic)
        {
            outDynamicStates.push_back(VK_DYNAMIC_STATE_STENCIL_REFERENCE);
        }

        outPDSCreateInfo.dynamicStateCount = outDynamicStates.size();
        outPDSCreateInfo.pDynamicStates = outDynamicStates.empty() ? nullptr : outDynamicStates.data();
    }
};
