/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#include "VulkanShader.h"
#include <Misc/Defines/VulkanProfilerDefines.h>
#include <Runtime/Graphics/VertexInputDescription.h>
#include <Runtime/File/FileHelper.h>
#include "VulkanDevice.h"
#include "VulkanTypeConverter.h"

#include <External/glslang/Include/glslang/SPIRV/GlslangToSpv.h>

/* ------------------------------------------------------------------------------- */
/* -----------------------          VulkanShader         ------------------------- */
/* ------------------------------------------------------------------------------- */

VulkanShader::VulkanShader(VulkanDevice* inDevice, EShaderType inShaderType)
{
    ShaderPool = nullptr;
    ShaderKey = UINT32_MAX;
    Device = inDevice;
    ShaderType = inShaderType;
    CompiledShaderBinary = nullptr;
    CompiledShaderBinarySize = 0;
}

VulkanShader::~VulkanShader() { }

void VulkanShader::BuildInputLayout(uint32 inNumVertexDescriptions, const FVertexInputDescription* inVertexDescriptions)
{
    if (inNumVertexDescriptions == 0 || inVertexDescriptions == nullptr) return;

    std::vector<VkVertexInputBindingDescription> BindingDescriptions;
    for (uint32 i = 0; i < inNumVertexDescriptions; ++i)
    {
        const FVertexInputDescription& Description = inVertexDescriptions[i];

        VkVertexInputBindingDescription InputBindingDescription = { };
        InputBindingDescription.binding = Description.BindingNum;
        InputBindingDescription.stride = Description.Stride;
        InputBindingDescription.inputRate = Description.InputRate == EInputRate::Vertex ? VK_VERTEX_INPUT_RATE_VERTEX : VK_VERTEX_INPUT_RATE_INSTANCE;

        InputBindings.push_back(InputBindingDescription);

        for (uint32 j = 0; j < Description.GetVertexAttributes().size(); j++)
        {
            const FVertexInputAttribute& Attribute = Description.GetVertexAttributes()[j];

            VkVertexInputAttributeDescription InputAttributeDescription = { };
            InputAttributeDescription.binding = Attribute.BindingNum;
            InputAttributeDescription.offset = Attribute.Offset;
            InputAttributeDescription.location = Attribute.Location;
            InputAttributeDescription.format = VulkanTypeConverter::Convert(Attribute.Format);

            InputAttributes.push_back(InputAttributeDescription);
        }
    }

    if (CompiledShaderBinary)
    {
        delete[] CompiledShaderBinary;
    }
}

void VulkanShader::CreateVertexInputStateCreateInfo(VkPipelineVertexInputStateCreateInfo& inInputStateCreateInfo) const
{
    inInputStateCreateInfo = VulkanUtils::Initializers::PipelineVertexInputStateCreateInfo();

    inInputStateCreateInfo.vertexBindingDescriptionCount = InputBindings.size();
    inInputStateCreateInfo.pVertexBindingDescriptions = InputBindings.data();

    inInputStateCreateInfo.vertexAttributeDescriptionCount = InputAttributes.size();
    inInputStateCreateInfo.pVertexAttributeDescriptions = InputAttributes.data();
}

void VulkanShader::ParseSpirvCodeIntoPipelineLayoutConfig(FPipelineLayoutConfig& outLayoutConfig) const
{
    uint32* SpirvCode = (uint32*)CompiledShaderBinary;
    uint32 SpirvCodeSize = CompiledShaderBinarySize / 4;

    uint32 MagicNumber = SpirvCode[0];
    VE_ASSERT(MagicNumber == spv::MagicNumber, VE_TEXT("[VulkanShader]: Cannot parse shader as its compiled binary code is corrupted..."));

    // Number of IDs that are contained within the binary file 
    uint32 BoundIds = SpirvCode[3];

    std::vector<FSpirvIdData> IdDatas;
    IdDatas.resize(BoundIds);

    // Word = uint32 | also spirv code is segmented into words meaning uint32 primitives
    uint32 WordIndex = 5;

    uint32 ShaderStageFlag = 0;
    while (WordIndex < SpirvCodeSize)
    {
        // Each ID definition starts with an Op type and the number of words it is
        // composed of [ OpType stored in bottom 16 bits while the word count is top 16 bits]
        spv::Op SpvOp = (spv::Op)(SpirvCode[WordIndex] & 0xFF);
        uint16 WordCount = (uint16)(SpirvCode[WordIndex] >> 16);

        switch (SpvOp)
        {

        case (spv::OpEntryPoint):
        {
            VE_ASSERT(WordCount >= 4, "[VulkanShader]: Invalid word count in spirv code dectected when parsing the compiled shader code...");

            spv::ExecutionModel ExecutionModel = (spv::ExecutionModel)SpirvCode[WordIndex + 1];

            ShaderStageFlag = ParseSpvExecutionModel(ExecutionModel);
            VE_ASSERT(ShaderType != EShaderType::Undefined, "[VulkanShader]: Invalid shader type was parsed from a compiled shader binary....");

            break;
        }

        /* Example Of OpDecorate in human readable spirv format
            OpDecorate %__0 DescriptorSet 0
            OpDecorate %__0 Binding 0
        */
        case (spv::OpDecorate):
        {
            VE_ASSERT(WordCount >= 3, "[VulkanShader]: Invalid word count in spirv code dectected when parsing the compiled shader code...");

            uint32 IdIndex = SpirvCode[WordIndex + 1];
            VE_ASSERT(IdIndex < BoundIds, "[VulkanShader]: Invalid ID Index detected when parsing a shaders compiled binary...");

            FSpirvIdData& IdData = IdDatas[IdIndex];

            spv::Decoration Decoration = (spv::Decoration)SpirvCode[WordIndex + 2];
            switch (Decoration)
            {
            case (spv::DecorationBinding):
            {
                IdData.Binding = SpirvCode[WordIndex + 3];
                break;
            }

            case (spv::DecorationDescriptorSet):
            {
                IdData.Set = SpirvCode[WordIndex + 3];
                break;
            }

            }

            break;
        }

        /* Example Of OpMemberDecorate in human readable spirv format
            OpMemberDecorate %LocalConstants 0 ColMajor
            OpMemberDecorate %LocalConstants 0 Offset 0
            OpMemberDecorate %LocalConstants 0 MatrixStride 16
            OpMemberDecorate %LocalConstants 1 ColMajor
            OpMemberDecorate %LocalConstants 1 Offset 64
            OpMemberDecorate %LocalConstants 1 MatrixStride 16
            OpMemberDecorate %LocalConstants 2 ColMajor
            OpMemberDecorate %LocalConstants 2 Offset 128
            OpMemberDecorate %LocalConstants 2 MatrixStride 16
            OpDecorate %LocalConstants Block
        */
        case (spv::OpMemberDecorate):
        {
            VE_ASSERT(WordCount >= 4, "[VulkanShader]: Invalid word count in spirv code dectected when parsing the compiled shader code...");

            uint32 IdIndex = SpirvCode[WordIndex + 1];
            VE_ASSERT(IdIndex < BoundIds, "[VulkanShader]: Invalid ID Index detected when parsing a shaders compiled binary...");

            FSpirvIdData& IdData = IdDatas[IdIndex];

            uint32 member_index = SpirvCode[WordIndex + 2];

            if (IdData.Members.capacity() == 0) {
                IdData.Members.resize(64);
            }

            FMemberField& MemberField = IdData.Members[member_index];

            spv::Decoration Decoration = (spv::Decoration)SpirvCode[WordIndex + 3];
            switch (Decoration)
            {
            case (spv::DecorationOffset):
            {
                MemberField.Offset = SpirvCode[WordIndex + 4];
                break;
            }
            }

            break;
        }
        /* Example Of OpName in human readable spirv format
            OpName %main "main"
        */
        case (spv::OpName):
        {
            VE_ASSERT(WordCount >= 3, "[VulkanShader]: Invalid word count in spirv code dectected when parsing the compiled shader code...");

            uint32 IdIndex = SpirvCode[WordIndex + 1];
            VE_ASSERT(IdIndex < BoundIds, "[VulkanShader]: Invalid ID Index detected when parsing a shaders compiled binary...");

            FSpirvIdData& IdData = IdDatas[IdIndex];

            char* Name = (char*)(SpirvCode + (WordIndex + 2));
            IdData.Name = Name;

            break;
        }


        /* Example Of OpMemberName in human readable spirv format
          OpName %LocalConstants "LocalConstants"
          OpMemberName% LocalConstants 0 "model"
          OpMemberName % LocalConstants 1 "view_projection"
          OpMemberName % LocalConstants 2 "model_inverse"
          OpName % __0 ""
         */
        case (spv::OpMemberName):
        {
            VE_ASSERT(WordCount >= 4, "[VulkanShader]: Invalid word count in spirv code dectected when parsing the compiled shader code...");

            uint32 IdIndex = SpirvCode[WordIndex + 1];
            VE_ASSERT(IdIndex < BoundIds, "[VulkanShader]: Invalid ID Index detected when parsing a shaders compiled binary...");

            FSpirvIdData& IdData = IdDatas[IdIndex];

            uint32 MemberFieldIndex = SpirvCode[WordIndex + 2];

            if (IdData.Members.capacity() == 0) {
                IdData.Members.resize(64);
            }

            FMemberField& MemberField = IdData.Members[MemberFieldIndex];

            char* Name = (char*)(SpirvCode + (WordIndex + 3));
            MemberField.Name = Name;

            break;
        }

        case (spv::OpTypeInt):
        {
            VE_ASSERT(WordCount == 4, "[VulkanShader]: Invalid word count in spirv code dectected when parsing the compiled shader code...");

            uint32 IdIndex = SpirvCode[WordIndex + 1];
            VE_ASSERT(IdIndex < BoundIds, "[VulkanShader]: Invalid ID Index detected when parsing a shaders compiled binary...");

            FSpirvIdData& IdData = IdDatas[IdIndex];
            IdData.SpvOp = SpvOp;
            IdData.Width = (uint8)SpirvCode[WordIndex + 2];
            IdData.Sign = (uint8)SpirvCode[WordIndex + 3];

            break;
        }

        case (spv::OpTypeFloat):
        {
            VE_ASSERT(WordCount == 3, "[VulkanShader]: Invalid word count in spirv code dectected when parsing the compiled shader code...");

            uint32 IdIndex = SpirvCode[WordIndex + 1];
            VE_ASSERT(IdIndex < BoundIds, "[VulkanShader]: Invalid ID Index detected when parsing a shaders compiled binary...");

            FSpirvIdData& IdData = IdDatas[IdIndex];
            IdData.SpvOp = SpvOp;
            IdData.Width = (uint8)SpirvCode[WordIndex + 2];

            break;
        }

        case (spv::OpTypeVector):
        {
            VE_ASSERT(WordCount == 4, "[VulkanShader]: Invalid word count in spirv code dectected when parsing the compiled shader code...");

            uint32 IdIndex = SpirvCode[WordIndex + 1];
            VE_ASSERT(IdIndex < BoundIds, "[VulkanShader]: Invalid ID Index detected when parsing a shaders compiled binary...");

            FSpirvIdData& IdData = IdDatas[IdIndex];
            IdData.SpvOp = SpvOp;
            IdData.TypeIndex = SpirvCode[WordIndex + 2];
            IdData.Count = SpirvCode[WordIndex + 3];

            break;
        }

        case (spv::OpTypeMatrix):
        {
            VE_ASSERT(WordCount == 4, "[VulkanShader]: Invalid word count in spirv code dectected when parsing the compiled shader code...");

            uint32 IdIndex = SpirvCode[WordIndex + 1];
            VE_ASSERT(IdIndex < BoundIds, "[VulkanShader]: Invalid ID Index detected when parsing a shaders compiled binary...");

            FSpirvIdData& IdData = IdDatas[IdIndex];
            IdData.SpvOp = SpvOp;
            IdData.TypeIndex = SpirvCode[WordIndex + 2];
            IdData.Count = SpirvCode[WordIndex + 3];

            break;
        }

        case (spv::OpTypeImage):
        {
            // NOTE(marco): not sure we need this information just yet
            VE_ASSERT(WordCount >= 9, "[VulkanShader]: Invalid word count in spirv code dectected when parsing the compiled shader code...");
            break;
        }

        case (spv::OpTypeSampler):
        {
            VE_ASSERT(WordCount == 2, "[VulkanShader]: Invalid word count in spirv code dectected when parsing the compiled shader code...");

            uint32 IdIndex = SpirvCode[WordIndex + 1];
            VE_ASSERT(IdIndex < BoundIds, "[VulkanShader]: Invalid ID Index detected when parsing a shaders compiled binary...");

            FSpirvIdData& IdData = IdDatas[IdIndex];
            IdData.SpvOp = SpvOp;

            break;
        }

        case (spv::OpTypeSampledImage):
        {
            VE_ASSERT(WordCount == 3, "[VulkanShader]: Invalid word count in spirv code dectected when parsing the compiled shader code...");

            uint32 IdIndex = SpirvCode[WordIndex + 1];
            VE_ASSERT(IdIndex < BoundIds, "[VulkanShader]: Invalid ID Index detected when parsing a shaders compiled binary...");

            FSpirvIdData& IdData = IdDatas[IdIndex];
            IdData.SpvOp = SpvOp;

            break;
        }

        case (spv::OpTypeArray):
        {
            VE_ASSERT(WordCount == 4, "[VulkanShader]: Invalid word count in spirv code dectected when parsing the compiled shader code...");

            uint32 IdIndex = SpirvCode[WordIndex + 1];
            VE_ASSERT(IdIndex < BoundIds, "[VulkanShader]: Invalid ID Index detected when parsing a shaders compiled binary...");

            FSpirvIdData& IdData = IdDatas[IdIndex];
            IdData.SpvOp = SpvOp;
            IdData.TypeIndex = SpirvCode[WordIndex + 2];
            IdData.Count = SpirvCode[WordIndex + 3];

            break;
        }

        case (spv::OpTypeRuntimeArray):
        {
            VE_ASSERT(WordCount == 3, "[VulkanShader]: Invalid word count in spirv code dectected when parsing the compiled shader code...");

            uint32 IdIndex = SpirvCode[WordIndex + 1];
            VE_ASSERT(IdIndex < BoundIds, "[VulkanShader]: Invalid ID Index detected when parsing a shaders compiled binary...");

            FSpirvIdData& IdData = IdDatas[IdIndex];
            IdData.SpvOp = SpvOp;
            IdData.TypeIndex = SpirvCode[WordIndex + 2];

            break;
        }

        case (spv::OpTypeStruct):
        {
            VE_ASSERT(WordCount >= 2, "[VulkanShader]: Invalid word count in spirv code dectected when parsing the compiled shader code...");

            uint32 IdIndex = SpirvCode[WordIndex + 1];
            VE_ASSERT(IdIndex < BoundIds, "[VulkanShader]: Invalid ID Index detected when parsing a shaders compiled binary...");

            FSpirvIdData& IdData = IdDatas[IdIndex];
            IdData.SpvOp = SpvOp;

            if (WordCount > 2) {
                for (uint16 memberIndex = 0; memberIndex < WordCount - 2; ++memberIndex) {
                    IdData.Members[memberIndex].IdIndex = SpirvCode[WordIndex + memberIndex + 2];
                }
            }

            break;
        }

        case (spv::OpTypePointer):
        {
            VE_ASSERT(WordCount == 4, "[VulkanShader]: Invalid word count in spirv code dectected when parsing the compiled shader code...");

            uint32 IdIndex = SpirvCode[WordIndex + 1];
            VE_ASSERT(IdIndex < BoundIds, "[VulkanShader]: Invalid ID Index detected when parsing a shaders compiled binary...");

            FSpirvIdData& IdData = IdDatas[IdIndex];
            IdData.SpvOp = SpvOp;
            IdData.TypeIndex = SpirvCode[WordIndex + 3];

            break;
        }

        case (spv::OpConstant):
        {
            VE_ASSERT(WordCount >= 4, "[VulkanShader]: Invalid word count in spirv code dectected when parsing the compiled shader code...");

            uint32 IdIndex = SpirvCode[WordIndex + 1];
            VE_ASSERT(IdIndex < BoundIds, "[VulkanShader]: Invalid ID Index detected when parsing a shaders compiled binary...");

            FSpirvIdData& IdData = IdDatas[IdIndex];
            IdData.SpvOp = SpvOp;
            IdData.TypeIndex = SpirvCode[WordIndex + 2];
            IdData.Value = SpirvCode[WordIndex + 3]; // NOTE(marco): we assume all constants to have maximum 32bit width

            break;
        }

        case (spv::OpVariable):
        {
            VE_ASSERT(WordCount >= 4, "[VulkanShader]: Invalid word count in spirv code dectected when parsing the compiled shader code...");

            uint32 IdIndex = SpirvCode[WordIndex + 2];
            VE_ASSERT(IdIndex < BoundIds, "[VulkanShader]: Invalid ID Index detected when parsing a shaders compiled binary...");

            FSpirvIdData& IdData = IdDatas[IdIndex];
            IdData.SpvOp = SpvOp;
            IdData.TypeIndex = SpirvCode[WordIndex + 1];
            IdData.SpvStorageClass = (spv::StorageClass)SpirvCode[WordIndex + 3];

            break;
        }

        }

        WordIndex += WordCount;
    }

    //outLayoutConfig = FPipelineLayoutConfig();
    for (uint32 idIndex = 0; idIndex < IdDatas.size(); ++idIndex)
    {
        FSpirvIdData& IdData = IdDatas[idIndex];

        if (IdData.SpvOp == spv::OpVariable)
        {
            switch (IdData.SpvStorageClass)
            {
            case spv::StorageClassUniform:
            case spv::StorageClassUniformConstant:
            {
                // Bindless bindings are handled by the renderer internally 
                // so we do not need to handle them here 
                if (IdData.Set == 1 && (IdData.Binding == 10 || IdData.Binding == 11))
                {
                    // Since the layout information gets aggregated by the render interface, no need to worry here about creating
                    // a new Pipeline Binding Descriptor 
                    outLayoutConfig.AddBindingDescriptorAt({ }, 1);
                    outLayoutConfig.NumSets = MathUtils::Max(IdData.Set + 1, outLayoutConfig.NumSets);
                    continue;
                }

                // Get the actual type here
                FSpirvIdData& UniformTypeData = IdDatas[IdDatas[IdData.TypeIndex].TypeIndex];

                FPipelineBinding Binding = { };
                Binding.BindingIndex = IdData.Binding;
                Binding.NumResources = 1;

                switch (UniformTypeData.SpvOp)
                {

                case spv::OpTypeStruct:
                {
                    Binding.ResourceType = EResourceType::Buffer;
                    Binding.BindFlags = FResourceBindFlags::UniformBuffer;
                    break;
                }

                case spv::OpTypeSampledImage:
                {
                    Binding.ResourceType = EResourceType::Texture;
                    Binding.BindFlags = FResourceBindFlags::Sampled;
                    break;
                }

                }

                // Get the binding descriptor were going to edit 
                FPipelineBindingDescriptor& BindingDescriptor = outLayoutConfig.GetBindingDescriptorAt(IdData.Set);

                // Set default stage flags to the one we parsed from current shader
                Binding.StageFlags = ShaderStageFlag;

                // Check if any other shader if parsed before this one has possibly had the same binding, if so just adjust 
                // the stage flags 
                if (BindingDescriptor.Bindings[Binding.BindingIndex].ResourceType != EResourceType::Undefined)
                {
                    Binding.StageFlags |= BindingDescriptor.Bindings[Binding.BindingIndex].StageFlags;
                }

                BindingDescriptor.AddBindingAt(Binding, Binding.BindingIndex);
                outLayoutConfig.NumSets = MathUtils::Max(IdData.Set + 1, outLayoutConfig.NumSets);
            }
            break;
            }
        }
    }
}

uint32 VulkanShader::ParseSpvExecutionModel(spv::ExecutionModel inModel) const
{
    switch (inModel)
    {
    case spv::ExecutionModelVertex:
        return FShaderStageFlags::VertexStage;
    case spv::ExecutionModelFragment:
        return FShaderStageFlags::FragmentStage;
    }

    return FShaderStageFlags::AllStage;
}

/* ------------------------------------------------------------------------------- */
/* -----------------------         TVulkanShader         ------------------------- */
/* ------------------------------------------------------------------------------- */

template<typename EShaderType TShaderType>
TVulkanShader<TShaderType>::TVulkanShader(VulkanDevice* inDevice)
    : VulkanShader(inDevice, TShaderType) { }

/* ------------------------------------------------------------------------------- */
/* -----------------------      VulkanShaderFactory      ------------------------- */
/* ------------------------------------------------------------------------------- */

TBuiltInResource VulkanShaderFactory::BuiltInResources;

VulkanShaderFactory::VulkanShaderFactory(VulkanDevice* inDevice/*, ResourceManager* inResourceManagerHandle*/)
{
    Device = inDevice;
    //ResourceManagerHandle = inResourceManagerHandle;

    {
        BuiltInResources.maxLights = 32;
        BuiltInResources.maxClipPlanes = 6;
        BuiltInResources.maxTextureUnits = 32;
        BuiltInResources.maxTextureCoords = 32;
        BuiltInResources.maxVertexAttribs = 64;
        BuiltInResources.maxVertexUniformComponents = 4096;
        BuiltInResources.maxVaryingFloats = 64;
        BuiltInResources.maxVertexTextureImageUnits = 32;
        BuiltInResources.maxCombinedTextureImageUnits = 80;
        BuiltInResources.maxTextureImageUnits = 32;
        BuiltInResources.maxFragmentUniformComponents = 4096;
        BuiltInResources.maxDrawBuffers = 32;
        BuiltInResources.maxVertexUniformVectors = 128;
        BuiltInResources.maxVaryingVectors = 8;
        BuiltInResources.maxFragmentUniformVectors = 16;
        BuiltInResources.maxVertexOutputVectors = 16;
        BuiltInResources.maxFragmentInputVectors = 15;
        BuiltInResources.minProgramTexelOffset = -8;
        BuiltInResources.maxProgramTexelOffset = 7;
        BuiltInResources.maxClipDistances = 8;
        BuiltInResources.maxComputeWorkGroupCountX = 65535;
        BuiltInResources.maxComputeWorkGroupCountY = 65535;
        BuiltInResources.maxComputeWorkGroupCountZ = 65535;
        BuiltInResources.maxComputeWorkGroupSizeX = 1024;
        BuiltInResources.maxComputeWorkGroupSizeY = 1024;
        BuiltInResources.maxComputeWorkGroupSizeZ = 64;
        BuiltInResources.maxComputeUniformComponents = 1024;
        BuiltInResources.maxComputeTextureImageUnits = 16;
        BuiltInResources.maxComputeImageUniforms = 8;
        BuiltInResources.maxComputeAtomicCounters = 8;
        BuiltInResources.maxComputeAtomicCounterBuffers = 1;
        BuiltInResources.maxVaryingComponents = 60;
        BuiltInResources.maxVertexOutputComponents = 64;
        BuiltInResources.maxGeometryInputComponents = 64;
        BuiltInResources.maxGeometryOutputComponents = 128;
        BuiltInResources.maxFragmentInputComponents = 128;
        BuiltInResources.maxImageUnits = 8;
        BuiltInResources.maxCombinedImageUnitsAndFragmentOutputs = 8;
        BuiltInResources.maxCombinedShaderOutputResources = 8;
        BuiltInResources.maxImageSamples = 0;
        BuiltInResources.maxVertexImageUniforms = 0;
        BuiltInResources.maxTessControlImageUniforms = 0;
        BuiltInResources.maxTessEvaluationImageUniforms = 0;
        BuiltInResources.maxGeometryImageUniforms = 0;
        BuiltInResources.maxFragmentImageUniforms = 8;
        BuiltInResources.maxCombinedImageUniforms = 8;
        BuiltInResources.maxGeometryTextureImageUnits = 16;
        BuiltInResources.maxGeometryOutputVertices = 256;
        BuiltInResources.maxGeometryTotalOutputComponents = 1024;
        BuiltInResources.maxGeometryUniformComponents = 1024;
        BuiltInResources.maxGeometryVaryingComponents = 64;
        BuiltInResources.maxTessControlInputComponents = 128;
        BuiltInResources.maxTessControlOutputComponents = 128;
        BuiltInResources.maxTessControlTextureImageUnits = 16;
        BuiltInResources.maxTessControlUniformComponents = 1024;
        BuiltInResources.maxTessControlTotalOutputComponents = 4096;
        BuiltInResources.maxTessEvaluationInputComponents = 128;
        BuiltInResources.maxTessEvaluationOutputComponents = 128;
        BuiltInResources.maxTessEvaluationTextureImageUnits = 16;
        BuiltInResources.maxTessEvaluationUniformComponents = 1024;
        BuiltInResources.maxTessPatchComponents = 120;
        BuiltInResources.maxPatchVertices = 32;
        BuiltInResources.maxTessGenLevel = 64;
        BuiltInResources.maxViewports = 16;
        BuiltInResources.maxVertexAtomicCounters = 0;
        BuiltInResources.maxTessControlAtomicCounters = 0;
        BuiltInResources.maxTessEvaluationAtomicCounters = 0;
        BuiltInResources.maxGeometryAtomicCounters = 0;
        BuiltInResources.maxFragmentAtomicCounters = 8;
        BuiltInResources.maxCombinedAtomicCounters = 8;
        BuiltInResources.maxAtomicCounterBindings = 1;
        BuiltInResources.maxVertexAtomicCounterBuffers = 0;
        BuiltInResources.maxTessControlAtomicCounterBuffers = 0;
        BuiltInResources.maxTessEvaluationAtomicCounterBuffers = 0;
        BuiltInResources.maxGeometryAtomicCounterBuffers = 0;
        BuiltInResources.maxFragmentAtomicCounterBuffers = 1;
        BuiltInResources.maxCombinedAtomicCounterBuffers = 1;
        BuiltInResources.maxAtomicCounterBufferSize = 16384;
        BuiltInResources.maxTransformFeedbackBuffers = 4;
        BuiltInResources.maxTransformFeedbackInterleavedComponents = 64;
        BuiltInResources.maxCullDistances = 8;
        BuiltInResources.maxCombinedClipAndCullDistances = 8;
        BuiltInResources.maxSamples = 4;
        BuiltInResources.maxMeshOutputVerticesNV = 256;
        BuiltInResources.maxMeshOutputPrimitivesNV = 512;
        BuiltInResources.maxMeshWorkGroupSizeX_NV = 32;
        BuiltInResources.maxMeshWorkGroupSizeY_NV = 1;
        BuiltInResources.maxMeshWorkGroupSizeZ_NV = 1;
        BuiltInResources.maxTaskWorkGroupSizeX_NV = 32;
        BuiltInResources.maxTaskWorkGroupSizeY_NV = 1;
        BuiltInResources.maxTaskWorkGroupSizeZ_NV = 1;
        BuiltInResources.maxMeshViewCountNV = 4;

        BuiltInResources.limits.nonInductiveForLoops = 1;
        BuiltInResources.limits.whileLoops = 1;
        BuiltInResources.limits.doWhileLoops = 1;
        BuiltInResources.limits.generalUniformIndexing = 1;
        BuiltInResources.limits.generalAttributeMatrixVectorIndexing = 1;
        BuiltInResources.limits.generalVaryingIndexing = 1;
        BuiltInResources.limits.generalSamplerIndexing = 1;
        BuiltInResources.limits.generalVariableIndexing = 1;
        BuiltInResources.limits.generalConstantMatrixVectorIndexing = 1;
    }
}

VulkanShaderFactory::~VulkanShaderFactory() { }

VulkanShader* VulkanShaderFactory::CreateShader(VulkanShaderPool* inShaderPool, const FShaderConfig& inConfig) const
{
    switch (inConfig.Type)
    {
    case EShaderType::Undefined:
        break;
    case EShaderType::Vertex:           return CreateVertexShader(inShaderPool, (FShaderConfig&)inConfig);
    case EShaderType::TessControl:
        break;
    case EShaderType::TessEvaluation:
        break;
    case EShaderType::Geometry:
        break;
    case EShaderType::Fragment:         return CreateFragmentShader(inShaderPool, (FShaderConfig&)inConfig);
    case EShaderType::Compute:
        break;
    default:
        break;
    }
    return nullptr;
}

VulkanVertexShader* VulkanShaderFactory::CreateVertexShader(VulkanShaderPool* inShaderPool, FShaderConfig& inConfig) const
{
    VulkanVertexShader* VertexShader = nullptr;

    if (inConfig.SourceType == EShaderSourceType::Filepath)
    {
        std::string ShaderSource;
        LoadShaderSourceFromFilePath(inConfig, ShaderSource);

        std::string FilePath = inConfig.SourceCode;

        inConfig.SourceCode = ShaderSource;
        inConfig.SourceType = EShaderSourceType::String;

        // Then we have to compile the shader only
        uint8* CompiledSourceCode = nullptr;
        uint64 CompiledSourceCodeSize = 0;
        CompileSourceCode(inConfig, CompiledSourceCode, &CompiledSourceCodeSize);

        VertexShader = new VulkanVertexShader(inShaderPool->Device);
        VertexShader->ShaderKey = inShaderPool->ShaderModuleHandles.size();
        VertexShader->ShaderPool = inShaderPool;

        // Create the shader module 
        inShaderPool->CreateShaderModule(CompiledSourceCode, CompiledSourceCodeSize);

        // Build the input layout information 
        VertexShader->BuildInputLayout(inConfig.VertexBindings.size(), inConfig.VertexBindings.data());

        // Free Resource -> 
        VertexShader->CompiledShaderBinary = CompiledSourceCode;
        VertexShader->CompiledShaderBinarySize = CompiledSourceCodeSize;
        VertexShader->Path = FilePath;

        if (inConfig.Flags & FShaderFlags::OutputBinary)
        {
            CreateShaderCompiledBinaryFile<EShaderType::Vertex>(VertexShader);
        }
    }
#if _DEBUG
    else
    {
        VE_ASSERT(false, VE_TEXT("[VulkanShaderFactory]: Cannot have an invalid shader source type..."));
    }
#endif

    return VertexShader;
}

VulkanFragmentShader* VulkanShaderFactory::CreateFragmentShader(VulkanShaderPool* inShaderPool, FShaderConfig& inConfig) const
{
    VulkanFragmentShader* FragmentShader = nullptr;

    if (inConfig.SourceType == EShaderSourceType::Filepath)
    {
        std::string ShaderSource;
        LoadShaderSourceFromFilePath(inConfig, ShaderSource);

        std::string FilePath = inConfig.SourceCode;

        inConfig.SourceCode = ShaderSource;
        inConfig.SourceType = EShaderSourceType::String;

        // Then we have to compile the shader only
        uint8* CompiledSourceCode = nullptr;
        uint64 CompiledSourceCodeSize = 0;
        CompileSourceCode(inConfig, CompiledSourceCode, &CompiledSourceCodeSize);

        FragmentShader = new VulkanFragmentShader(inShaderPool->Device);
        FragmentShader->ShaderKey = inShaderPool->ShaderModuleHandles.size();
        FragmentShader->ShaderPool = inShaderPool;

        // Create the shader module 
        inShaderPool->CreateShaderModule(CompiledSourceCode, CompiledSourceCodeSize);

        // Free Resource -> 
        FragmentShader->CompiledShaderBinary = CompiledSourceCode;
        FragmentShader->CompiledShaderBinarySize = CompiledSourceCodeSize;
        FragmentShader->Path = FilePath;

        if (inConfig.Flags & FShaderFlags::OutputBinary)
        {
            CreateShaderCompiledBinaryFile<EShaderType::Fragment>(FragmentShader);
        }
    }
#if _DEBUG
    else
    {
        VE_ASSERT(false, VE_TEXT("[VulkanShaderFactory]: Cannot have an invalid shader source type..."));
    }
#endif

    return FragmentShader;
}

void VulkanShaderFactory::CompileSourceCode(const FShaderConfig& inConfig, uint8*& outCode, uint64* outCodeSize) const
{
    glslang::InitializeProcess();

    EShLanguage ShaderStage = ConvertShaderType(inConfig.Type);
    glslang::EShSource ShaderSourceLanguage = glslang::EShSourceHlsl;

    if (inConfig.Flags & FShaderFlags::GLSL)
    {
        ShaderSourceLanguage = glslang::EShSourceGlsl;
    }

    glslang::TShader RawShader(ShaderStage);

    const char* RawInput = inConfig.SourceCode.c_str();
    const char* const* CStrInput = &RawInput;
    RawShader.setStrings(CStrInput, 1);

    int ClientInputSemanticsVersion = 100;
    glslang::EShTargetClientVersion VulkanClientVersion = glslang::EShTargetVulkan_1_0;
    glslang::EShTargetLanguageVersion TargetVersion = glslang::EShTargetSpv_1_0;

    RawShader.setEnvInput(ShaderSourceLanguage, ShaderStage, glslang::EShClientVulkan, ClientInputSemanticsVersion);
    RawShader.setEnvClient(glslang::EShClientVulkan, VulkanClientVersion);
    RawShader.setEnvTarget(glslang::EShTargetSpv, TargetVersion);

    // Since DefaultTBuiltInResource get reinterpert casted to glslang_resource_t* we can just do some 
    // pointer casting to get the original values back 
    EShMessages messages = EShMsgDefault;

    const int DefaultVersion = 100;

    // preprocessing of the glsl shader (includes all files into the actual glsl string)
    glslang::TShader::ForbidIncluder BasicIncluder = glslang::TShader::ForbidIncluder();
    std::string preprocessedGLSL;
    if (!RawShader.preprocess(&BuiltInResources, DefaultVersion, ENoProfile, false, false, messages, &preprocessedGLSL, BasicIncluder)) {
        VE_ASSERT(false, "Shader Preprocessing Failed \n Log: {0}\n DebugLog: {1}", RawShader.getInfoLog(), RawShader.getInfoDebugLog());
    }

    //updates shader strings with preprocessed glsl file
    const char* preprocessedCStr = preprocessedGLSL.c_str();
    RawShader.setStrings(&preprocessedCStr, 1);

    //parses the shader
    if (!RawShader.parse(&BuiltInResources, 100, false, messages)) {
        VE_ASSERT(false, "Shader Parsing Failed \n Log: {0}\n DebugLog: {1}", RawShader.getInfoLog(), RawShader.getInfoDebugLog());
    }

    glslang::TProgram program;
    program.addShader(&RawShader);

    //links program with shader
    if (!program.link(messages)) {
        VE_ASSERT(false, "Shader Linking \n Log: {0}\n DebugLog: {1}", RawShader.getInfoLog(), RawShader.getInfoDebugLog());
    }

    //converts glslang program to spirv format
    std::vector<uint32> SpirV;
    spv::SpvBuildLogger logger;
    glslang::SpvOptions spvOptions;
    spvOptions.validate = true;
    glslang::GlslangToSpv(*program.getIntermediate(ShaderStage), SpirV, &logger, &spvOptions);

    if (logger.getAllMessages().size() > 0)
    {
        VE_CORE_LOG_WARN("[VulkanShaderFactory]: Spirv Messages: \n {0}", logger.getAllMessages());
    }

    *outCodeSize = SpirV.size() * 4;
    outCode = new uint8[*outCodeSize];
    memcpy(outCode, SpirV.data(), *outCodeSize);

    {
        // Create the compilers input structure 
        /*glslang_stage_t ShaderStage = ConvertShaderType(inConfig.Type);
        glslang_source_t ShaderSourceLanguage = GLSLANG_SOURCE_HLSL;

        if (inConfig.Flags & FShaderFlags::GLSL)
        {
            ShaderSourceLanguage = GLSLANG_SOURCE_GLSL;
        }*/
        //const glslang_input_t ShaderCreationInput = {
        //    ShaderSourceLanguage,
        //    ShaderStage,
        //    GLSLANG_CLIENT_VULKAN,
        //    GLSLANG_TARGET_VULKAN_1_1,
        //    GLSLANG_TARGET_SPV,
        //    GLSLANG_TARGET_SPV_1_3,
        //    inConfig.SourceCode.data(),
        //    100,
        //    GLSLANG_NO_PROFILE,
        //    false,
        //    false,
        //    GLSLANG_MSG_DEFAULT_BIT,
        //    glslang_default_resource()
        //};

        //// Create the shader with teh ShaderCreationInput structure we just created  
        //glslang_shader_t* RawShader = glslang_shader_create(&ShaderCreationInput);

        //// Firstly the shader has to go through the preprocess phase by the compiler 
        //if (!glslang_shader_preprocess(RawShader, &ShaderCreationInput))
        //{
        //    VE_ASSERT(false, "Failed to Compile Shader \n\n {0} \n\n {1}",
        //        glslang_shader_get_info_log(RawShader), glslang_shader_get_info_debug_log(RawShader));
        //    return;
        //}

        //// Next allows the compiler to parse the shader into an internal parse tree representation
        //if (!glslang_shader_parse(RawShader, &ShaderCreationInput))
        //{
        //    VE_ASSERT(false, "Failed to Parse Compiled Shader \n\n {0} \n\n {1} \n\n Preprocessed Code: \n {2}",
        //        glslang_shader_get_info_log(RawShader), glslang_shader_get_info_debug_log(RawShader),
        //        glslang_shader_get_preprocessed_code(RawShader));
        //    return;
        //}

        //// Next we can link the shader to a program and generate the binary code 
        //glslang_program_t* Program = glslang_program_create();
        //glslang_program_add_shader(Program, RawShader);

        //if (!glslang_program_link(Program, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT))
        //{
        //    VE_ASSERT(false, "Failed to Link Shader \n\n {0} \n\n {1}",
        //        glslang_program_get_info_log(Program), glslang_program_get_info_debug_log(Program));
        //    return;
        //}

        //// Next Generate SPIRV Code 
        //glslang_program_SPIRV_generate(Program, ShaderStage);

        //// Get Code Size
        //*outCodeSize = glslang_program_SPIRV_get_size(Program);

        //// Resize then get the code 
        //outCode = new uint32[*outCodeSize];
        //glslang_program_SPIRV_get(Program, outCode);

        //{
        //    const char* SPIRV_Messages = glslang_program_SPIRV_get_messages(Program);

        //    if (SPIRV_Messages)
        //    {
        //        VE_CORE_LOG_WARN("[VulkanShaderFactory]: Spirv Messages: \n {0}", SPIRV_Messages);
        //    }
        //}

        //glslang_program_delete(Program);
        //glslang_shader_delete(RawShader);
    }

    //glslang_finalize_process();
    glslang::FinalizeProcess();
}

EShLanguage VulkanShaderFactory::ConvertShaderType(EShaderType inShaderType) const
{
    switch (inShaderType)
    {
    case EShaderType::Vertex:
        return EShLangVertex;
    case EShaderType::Fragment:
        return EShLangFragment;
    }

    VE_ASSERT(false, VE_TEXT("[VulkanShaderFactory]: Shader type currently not supported or is undefined: {0}"), (int32)inShaderType);
    return EShLangVertex;
}

void VulkanShaderFactory::LoadShaderSourceFromFilePath(const FShaderConfig& inConfig, std::string& outSource) const
{
    FileHelper::LoadFileToString(outSource, (std::string&)inConfig.SourceCode);
}

template<EShaderType T>
inline void VulkanShaderFactory::CreateShaderCompiledBinaryFile(const TVulkanShader<T>* inVulkanShader) const
{
    VE_ASSERT(inVulkanShader->CompiledShaderBinary != nullptr
        || inVulkanShader->CompiledShaderBinarySize != 0
        || inVulkanShader->Path.size() != 0,
        VE_TEXT("[VulkanShaderFactory]: Cannot create a compiled binary file for a shader: {0}"), inVulkanShader->Path);

    uint32 LastSlashIndex = inVulkanShader->Path.find_last_of('/');
    uint32 LastPeriodIndex = inVulkanShader->Path.find_last_of('.');

    std::string FileName = inVulkanShader->Path.substr(LastSlashIndex, LastPeriodIndex - LastSlashIndex);
    std::string PathToFolder = inVulkanShader->Path.substr(0, LastSlashIndex);

    std::string OutputFile = PathToFolder + FileName;
    if (T == EShaderType::Vertex)
    {
        OutputFile += "_vert.spv";
    }
    else if (T == EShaderType::Fragment)
    {
        OutputFile += "_frag.spv";
    }

    std::ofstream FileHandle;
    FileHandle.open(OutputFile, std::ios::ate);

    if (FileHandle.is_open())
    {
        FileHandle.write((const char*)(inVulkanShader->CompiledShaderBinary), inVulkanShader->CompiledShaderBinarySize);
    }

    FileHandle.close();
}

VulkanShaderPool::VulkanShaderPool(VulkanDevice* inDevice)
    : Device(inDevice) { }

VulkanShaderPool::~VulkanShaderPool()
{
    Device->WaitUntilIdle();

    for (uint32 i = 0; i < ShaderModuleHandles.size(); i++)
    {
        vkDestroyShaderModule(*Device->GetDeviceHandle(), ShaderModuleHandles[i], nullptr);
    }
}

VkShaderModule VulkanShaderPool::CreateShaderModule(const uint8* inCompiledShaderCode, uint64 inCodeSize)
{
    VkShaderModuleCreateInfo ShaderModuleCreateInfo = {};
    ShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    ShaderModuleCreateInfo.codeSize = inCodeSize;
    ShaderModuleCreateInfo.pCode = (uint32*)inCompiledShaderCode;

    //Create the Shader Module and return the result of it.
    VkShaderModule ShaderModuleHandle = VK_NULL_HANDLE;
    VK_CHECK_RESULT(vkCreateShaderModule(*Device->GetDeviceHandle(), &ShaderModuleCreateInfo, nullptr, &ShaderModuleHandle), "[VulkanShaderPool]: Failed to create a shader module from compiled shader source code!!");

    ShaderModuleHandles.push_back(ShaderModuleHandle);
    return ShaderModuleHandle;
}
