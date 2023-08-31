#include "FrameGraph.h"

#include <Runtime/File/FileHelper.h>
#include <Runtime/Graphics/Renderer.h>

#include <External/json/Includes/nlohmann_json/json.hpp>

namespace FrameGraphHelpers
{
    static EFrameGraphResourceType StringToResourceType(const std::string& inInputType)
    {
        if (strcmp(inInputType.c_str(), "texture") == 0)
        {
            return EFrameGraphResourceType::Texture;
        }

        if (strcmp(inInputType.c_str(), "attachment") == 0)
        {
            return EFrameGraphResourceType::Attachment;
        }

        if (strcmp(inInputType.c_str(), "buffer") == 0)
        {
            return EFrameGraphResourceType::Buffer;
        }

        if (strcmp(inInputType.c_str(), "reference") == 0)
        {
            return EFrameGraphResourceType::Reference;
        }

        VE_ASSERT(false, VE_TEXT("[FrameGraphHelpers]: Error: cannot identify resource type from string..."))
            return EFrameGraphResourceType::Invalid;
    }

    static EPixelFormat StringToPixelFormat(const std::string& inInputType)
    {
#define ComparePixelFormat(x) if(strcmp(inInputType.c_str(), #x) == 0) { return EPixelFormat::x; }

        ComparePixelFormat(A8UNorm)
            ComparePixelFormat(R8UNorm)
            ComparePixelFormat(R8SNorm)
            ComparePixelFormat(R8UInt)
            ComparePixelFormat(R8SInt)
            ComparePixelFormat(R8SRGB)
            ComparePixelFormat(R16UNorm)
            ComparePixelFormat(R16SNorm)
            ComparePixelFormat(R16UInt)
            ComparePixelFormat(R16SInt)
            ComparePixelFormat(R16Float)
            ComparePixelFormat(R32UInt)
            ComparePixelFormat(R32SInt)
            ComparePixelFormat(R32Float)
            ComparePixelFormat(R64Float)
            ComparePixelFormat(RG8UNorm)
            ComparePixelFormat(RG8SNorm)
            ComparePixelFormat(RG8UInt)
            ComparePixelFormat(RG8SInt)
            ComparePixelFormat(RG16UNorm)
            ComparePixelFormat(RG16SNorm)
            ComparePixelFormat(RG16UInt)
            ComparePixelFormat(RG16SInt)
            ComparePixelFormat(RG16Float)
            ComparePixelFormat(RG32UInt)
            ComparePixelFormat(RG32SInt)
            ComparePixelFormat(RG32Float)
            ComparePixelFormat(RG64Float)
            ComparePixelFormat(RGB8UNorm)
            ComparePixelFormat(RGB8UNorm_sRGB)
            ComparePixelFormat(RGB8SNorm)
            ComparePixelFormat(RGB8UInt)
            ComparePixelFormat(RGB8SInt)
            ComparePixelFormat(RGB16UNorm)
            ComparePixelFormat(RGB16SNorm)
            ComparePixelFormat(RGB16UInt)
            ComparePixelFormat(RGB16SInt)
            ComparePixelFormat(RGB16Float)
            ComparePixelFormat(RGB32UInt)
            ComparePixelFormat(RGB32SInt)
            ComparePixelFormat(RGB32Float)
            ComparePixelFormat(RGB64Float)
            ComparePixelFormat(RGBA8UNorm)
            ComparePixelFormat(RGBA8UNorm_sRGB)
            ComparePixelFormat(RGBA8SNorm)
            ComparePixelFormat(RGBA8UInt)
            ComparePixelFormat(RGBA8SInt)
            ComparePixelFormat(RGBA16UNorm)
            ComparePixelFormat(RGBA16SNorm)
            ComparePixelFormat(RGBA16UInt)
            ComparePixelFormat(RGBA16SInt)
            ComparePixelFormat(RGBA16Float)
            ComparePixelFormat(RGBA32UInt)
            ComparePixelFormat(RGBA32SInt)
            ComparePixelFormat(RGBA32Float)
            ComparePixelFormat(RGBA64Float)
            ComparePixelFormat(BGRA8UNorm)
            ComparePixelFormat(BGRA8UNorm_sRGB)
            ComparePixelFormat(BGRA8SNorm)
            ComparePixelFormat(BGRA8UInt)
            ComparePixelFormat(BGRA8SInt)
            ComparePixelFormat(D16UNorm)
            ComparePixelFormat(D24UNormS8UInt)
            ComparePixelFormat(D32Float)
            ComparePixelFormat(D32FloatS8X24UInt)
            ComparePixelFormat(S8UInt)

            VE_ASSERT(false, VE_TEXT("[FrameGraphHelpers]: Error: cannot identify pixel format from string..."))
            return EPixelFormat::Undefined;
    }

    static EAttachmentLoadOp StringToRenderPassOp(const std::string& inInputType)
    {
        if (strcmp(inInputType.c_str(), "LoadOpClear") == 0)
        {
            return EAttachmentLoadOp::Clear;
        }
        else if (strcmp(inInputType.c_str(), "LoadOpLoad") == 0)
        {
            return EAttachmentLoadOp::Load;
        }

        VE_ASSERT(false, VE_TEXT("[FrameGraphHelpers]: Error: cannot identify renderpass Load Operation from string..."))
            return EAttachmentLoadOp::Undefined;
    }

    static bool HasDepthOrStencil(EPixelFormat inFormat)
    {
        return inFormat >= EPixelFormat::D16UNorm && inFormat <= EPixelFormat::D32FloatS8X24UInt;
    }
}

void FrameGraph::Init(FrameGraphBuilder* inBuilder)
{
    GraphBuilder = inBuilder;
}

void FrameGraph::Shutdown()
{
}

void FrameGraph::Parse(const std::string inFilePath)
{
    using json = nlohmann::json;

    if (!FileHelper::DoesFileExist(inFilePath))
    {
        VE_CORE_LOG_ERROR(VE_TEXT("[FrameGraph]: Cannot find specified file: {0}"), inFilePath);
        return;
    }

    std::string Result;
    if (!FileHelper::LoadFileToString(Result, inFilePath))
    {
        VE_CORE_LOG_ERROR(VE_TEXT("[FrameGraph]: Cannot load file: {0}"), inFilePath);
        return;
    }

    json GraphData = json::parse(Result);

    // Get the graph name 
    Name = GraphData.value("name", "");

    // get all the render passes
    json RenderPasses = GraphData["passes"];

    // Parse the nodes 
    for (uint64 i = 0; i < RenderPasses.size(); ++i)
    {
        json RenderPass = RenderPasses[i];

        json PassInputs = RenderPass["inputs"];
        json PassOutputs = RenderPass["outputs"];

        FFrameGraphNodeCreation NodeCreation = { };
        NodeCreation.Inputs.resize(PassInputs.size());
        NodeCreation.Outputs.resize(PassOutputs.size());

        // Parse the inputs
        for (uint64 inputIndex = 0; inputIndex < PassInputs.size(); ++inputIndex)
        {
            json PassInput = PassInputs[inputIndex];

            FFrameGraphResourceInputCreation InputCreation = { };

            InputCreation.Name = PassInput.value("name", "");
            InputCreation.Type = FrameGraphHelpers::StringToResourceType(PassInput.value("type", ""));
            InputCreation.ResourceInfo.bIsExternalResource = false;

            VE_ASSERT(!InputCreation.Name.empty(), VE_TEXT("[FrameGraph]: Error parsing a Render pass Input... no name provided..."));
            NodeCreation.Inputs[inputIndex] = InputCreation;
        }

        // Parse the outputs 
        for (uint64 outputIndex = 0; outputIndex < PassOutputs.size(); ++outputIndex)
        {
            json PassOutput = PassOutputs[outputIndex];

            FFrameGraphResourceOutputCreation OutputCreation = { };

            OutputCreation.Name = PassOutput.value("name", "");
            OutputCreation.Type = FrameGraphHelpers::StringToResourceType(PassOutput.value("type", ""));

            switch (OutputCreation.Type)
            {
            case EFrameGraphResourceType::Attachment:
            case EFrameGraphResourceType::Texture:
            {
                OutputCreation.ResourceInfo.TextureResourceInfo.Format = FrameGraphHelpers::StringToPixelFormat(PassOutput.value("format", ""));
                OutputCreation.ResourceInfo.TextureResourceInfo.LoadOp = FrameGraphHelpers::StringToRenderPassOp(PassOutput.value("op", ""));

                json Resolution = PassOutput["resolution"];
                OutputCreation.ResourceInfo.TextureResourceInfo.Width = Resolution[0];
                OutputCreation.ResourceInfo.TextureResourceInfo.Height = Resolution[1];
                OutputCreation.ResourceInfo.TextureResourceInfo.Depth = 1;

                break;
            }
            case EFrameGraphResourceType::Buffer:
            {
                // still have to do this
                VE_ASSERT(false, VE_TEXT(""));
            }
            }

            VE_ASSERT(!OutputCreation.Name.empty(), VE_TEXT("[FrameGraph]: Error parsing a Render pass Output... no name provided..."));
            NodeCreation.Outputs[outputIndex] = OutputCreation;
        }

        NodeCreation.Name = RenderPass.value("name", "");
        NodeCreation.bIsEnabled = RenderPass.value("enabled", true);

        FFrameGraphNode* Node = GraphBuilder->CreateNode(NodeCreation);
        Nodes.push_back(Node);
    }
}

void FrameGraph::Reset()
{
}

void FrameGraph::EnableRenderPass(std::string inRenderPassName)
{
    FFrameGraphNode* Node = GraphBuilder->GetNode(inRenderPassName);
    Node->bIsEnabled = true;
}

void FrameGraph::DisableRenderPass(std::string inRenderPassName)
{
    FFrameGraphNode* Node = GraphBuilder->GetNode(inRenderPassName);
    Node->bIsEnabled = false;
}

void FrameGraph::Compile()
{
    // @TODO cull inactive nodes 

    for (uint32 i = 0; i < Nodes.size(); ++i)
    {
        FFrameGraphNode* Node = Nodes[i];
        Node->Edges.clear();
    }

    for (uint32 i = 0; i < Nodes.size(); ++i)
    {
        FFrameGraphNode* Node = Nodes[i];
        if (!Node->bIsEnabled)
        {
            continue;
        }

        ComputeEdges(this, Node, i);
    }

    std::vector<FFrameGraphNode*> SortedNodes;
    SortedNodes.resize(Nodes.size());
    uint32 NodesIndex = 0;

    TMap<FFrameGraphNode*, uint8> VisitedMap;

    std::vector<FFrameGraphNode*> Stack;
    //Stack.resize(Nodes.size());

    // Topological sorting
    for (uint32 n = 0; n < Nodes.size(); ++n)
    {
        FFrameGraphNode* Node = Nodes[n];
        if (!Node->bIsEnabled)
        {
            continue;
        }

        Stack.push_back(Node);

        while (Stack.size() > 0)
        {
            FFrameGraphNode* NodeHandle = Stack.back();

            auto It = VisitedMap.Find(NodeHandle);
            if (It != VisitedMap.End())
            {
                if (It->second == 2)
                {
                    Stack.pop_back();
                    continue;
                }

                //if (It->second == 1)
                //{
                //    It->second = 2; // added
                //    SortedNodes[NodesIndex++] = NodeHandle;
                //    Stack.pop_back();

                //    continue;
                //}

                It->second = 2; // added
                SortedNodes[NodesIndex++] = NodeHandle;
                Stack.pop_back();

                continue;
            }

            VisitedMap.Add(NodeHandle, 1); // Visited

            // Leaf Node
            if (NodeHandle->Edges.size() == 0)
            {
                continue;
            }

            for (uint32 r = 0; r < NodeHandle->Edges.size(); ++r)
            {
                FFrameGraphNode* ChildNode = NodeHandle->Edges[r];

                if (VisitedMap.Find(ChildNode) == VisitedMap.End())
                {
                    Stack.push_back(ChildNode);
                }
            }
        }
    }

    VE_ASSERT(SortedNodes.size() == Nodes.size(), VE_TEXT(""));

    Nodes.clear();
    Nodes.resize(SortedNodes.size());
    NodesIndex = 0;
    for (int32 i = SortedNodes.size() - 1; i >= 0; --i)
    {
        Nodes[NodesIndex++] = SortedNodes[i];
    }

    // Memory aliasing (Resource Aliasing) technique computation
    // 
    // @note allocations and deallocations are used for verification purposes only 
    uint64 ResourceCount = GraphBuilder->ResourceMap.Count();

    // tracks which node a given resource was allocated
    TMap<FFrameGraphResource*, FFrameGraphNode*> Allocations;

    // tracks the node the resource can be deallocated 
    TMap<FFrameGraphResource*, FFrameGraphNode*> Deallocations;

    // contains the resources that have been freed and can be reused 
    std::vector<TextureResource*> FreeList;

    uint64 PeakMemory = 0;
    uint64 InstantMemory = 0;

    // Update resources reference counts each time they are used as inputs 
    for (uint32 i = 0; i < Nodes.size(); ++i)
    {
        FFrameGraphNode* Node = Nodes[i];
        if (!Node->bIsEnabled)
        {
            continue;
        }

        for (uint32 j = 0; j < Node->Inputs.size(); ++j)
        {
            FFrameGraphResource* InputResource = Node->Inputs[j];
            FFrameGraphResource* Resource = InputResource->OutputHandle;

            Resource->ReferenceCount++;
        }
    }

    for (uint32 i = 0; i < Nodes.size(); ++i)
    {
        FFrameGraphNode* Node = Nodes[i];
        if (!Node->bIsEnabled)
        {
            continue;
        }

        for (uint32 j = 0; j < Node->Outputs.size(); ++j)
        {
            FFrameGraphResource* Resource = Node->Outputs[j];

            auto It = Allocations.Find(Resource);
            if (!Resource->ResourceInfo.bIsExternalResource && It == Allocations.End())
            {
                VE_ASSERT(Deallocations.Find(Resource) == Deallocations.End(), VE_TEXT("Error on frame graph compilation occurred..."));
                Allocations.Add(Resource, Node);

                if (Resource->Type == EFrameGraphResourceType::Attachment)
                {
                    FFrameGraphResourceInfo& Info = Resource->ResourceInfo;

                    FTextureConfig Config = { };
                    {
                        Config.Format = Info.TextureResourceInfo.Format;
                        Config.Type = ETextureType::Texture2D;

                        Config.Extent.Width = Info.TextureResourceInfo.Width;
                        Config.Extent.Height = Info.TextureResourceInfo.Height;
                        Config.Extent.Depth = Info.TextureResourceInfo.Depth;

                        Config.BindFlags |= FResourceBindFlags::ColorAttachment;

                        if (FrameGraphHelpers::HasDepthOrStencil(Config.Format))
                        {
                            Config.BindFlags = 0;
                            Config.BindFlags |= FResourceBindFlags::DepthStencilAttachment;
                        }
                    }

                    if (FreeList.size() > 0)
                    {
                        // @TODO: find best fit (currently its greedy)
                        TextureResource*& AliasTexture = FreeList.back();
                        FreeList.pop_back();

                        Config.TextureHandle = AliasTexture;

                        Info.TextureResourceInfo.TextureHandle = Renderer::Get().GetRenderInterface().Get()->CreateTexture(Config);
                    }
                    else
                    {
                        Info.TextureResourceInfo.TextureHandle = Renderer::Get().GetRenderInterface().Get()->CreateTexture(Config);
                    }
                }

                VE_CORE_LOG_INFO(VE_TEXT("[FrameGraph]: Renderpass Ouput {0} allocated on node {1}"), Resource->Name, Node->Name);
            }
        }

        for (uint32 j = 0; j < Node->Inputs.size(); ++j)
        {
            FFrameGraphResource* InputResource = Node->Inputs[j];
            FFrameGraphResource* Resource = InputResource->OutputHandle;

            Resource->ReferenceCount--;

            if (!Resource->ResourceInfo.bIsExternalResource && Resource->ReferenceCount == 0)
            {
                VE_ASSERT(Deallocations.Find(Resource) == Deallocations.End(), VE_TEXT("error on frame graph compile reference count check post..."));
                Deallocations.Add(Resource, Node);

                if (Resource->Type == EFrameGraphResourceType::Attachment || Resource->Type == EFrameGraphResourceType::Texture)
                {
                    FreeList.push_back(Resource->ResourceInfo.TextureResourceInfo.TextureHandle);
                }

                VE_CORE_LOG_INFO(VE_TEXT("[FrameGraph]: Renderpass Ouput {0} deallocated on node {1}"), Resource->Name, Node->Name);
            }
        }
    }

    for (uint32 i = 0; i < Nodes.size(); ++i)
    {
        FFrameGraphNode* Node = Nodes[i];
        if (!Node->bIsEnabled)
        {
            continue;
        }

        if (Node->RenderPassHandle == nullptr)
        {
            CreateRenderPass(this, Node);
        }

        if (Node->FrameBufferHandle == nullptr)
        {
            CreateFrameBuffer(this, Node);
        }
    }
}

FFrameGraphNode* FrameGraph::GetNode(std::string& inName)
{
    return GraphBuilder->GetNode(inName);
}

FFrameGraphResource* FrameGraph::GetResource(std::string& inName)
{
    return GraphBuilder->GetResource(inName);
}

void FrameGraph::ComputeEdges(FrameGraph* inFrameGraph, FFrameGraphNode* inNode, uint32 inNodeIndex)
{
    for (uint32 resourceIndex = 0; resourceIndex < inNode->Inputs.size(); ++resourceIndex)
    {
        FFrameGraphResource* Resource = inNode->Inputs[resourceIndex];
        FFrameGraphResource* OutputResource = inFrameGraph->GetResource(Resource->Name);
        if (OutputResource == nullptr && !Resource->ResourceInfo.bIsExternalResource)
        {
            VE_ASSERT(false, VE_TEXT("[FrameGraphHelpers]: Requested resource is not produced by any node and is not external..."));
            continue;
        }

        Resource->Producer = OutputResource->Producer;
        Resource->ResourceInfo = OutputResource->ResourceInfo;
        Resource->OutputHandle = OutputResource->OutputHandle;

        FFrameGraphNode* ParentNode = Resource->Producer;
        ParentNode->Edges.push_back(inFrameGraph->Nodes[inNodeIndex]);
    }
}

void FrameGraph::CreateRenderPass(FrameGraph* inFrameGraph, FFrameGraphNode* inNode)
{
    FRenderPassConfig RenderPassConfig = { };

    // @note firstly we want to create the outputs
    for (uint64 i = 0; i < inNode->Outputs.size(); ++i)
    {
        FFrameGraphResource* OutputResource = inNode->Outputs[i];
        FFrameGraphResourceInfo& Info = OutputResource->ResourceInfo;

        if (OutputResource->Type == EFrameGraphResourceType::Attachment)
        {
            if (Info.TextureResourceInfo.Format == EPixelFormat::D24UNormS8UInt)
            {
                RenderPassConfig.DepthStencilAttachment.Format = EPixelFormat::D24UNormS8UInt;
                RenderPassConfig.DepthStencilAttachment.FinalLayout = ETextureLayout::DepthStencilAttachment;
                RenderPassConfig.DepthStencilAttachment.LoadOp = EAttachmentLoadOp::Clear;

                continue;
            }

            FAttachmentDescription Desc = { };
            Desc.Format = Info.TextureResourceInfo.Format;
            Desc.FinalLayout = ETextureLayout::ColorAttachment;
            Desc.LoadOp = Info.TextureResourceInfo.LoadOp;

            RenderPassConfig.ColorAttachments.push_back(Desc);
        }
    }

    for (uint64 i = 0; i < inNode->Inputs.size(); ++i)
    {
        FFrameGraphResource* InputResource = inNode->Inputs[i];
        FFrameGraphResourceInfo& Info = InputResource->ResourceInfo;

        if (InputResource->Type == EFrameGraphResourceType::Attachment)
        {
            if (Info.TextureResourceInfo.Format == EPixelFormat::D24UNormS8UInt)
            {
                RenderPassConfig.DepthStencilAttachment.Format = EPixelFormat::D24UNormS8UInt;
                RenderPassConfig.DepthStencilAttachment.FinalLayout = ETextureLayout::DepthStencilAttachment;
                RenderPassConfig.DepthStencilAttachment.LoadOp = EAttachmentLoadOp::Load;

                continue;
            }

            FAttachmentDescription Desc = { };
            Desc.Format = Info.TextureResourceInfo.Format;
            Desc.FinalLayout = ETextureLayout::ColorAttachment;
            Desc.LoadOp = EAttachmentLoadOp::Load;

            RenderPassConfig.ColorAttachments.push_back(Desc);
        }
    }

    // @TODO: insure formats are valid for attachments 
    inNode->RenderPassHandle = Renderer::Get().GetRenderInterface().Get()->CreateRenderPass(RenderPassConfig);
}

void FrameGraph::CreateFrameBuffer(FrameGraph* inFrameGraph, FFrameGraphNode* inNode)
{
    FFrameBufferConfig FrameBufferConfig = { };

    FrameBufferConfig.RenderPass = inNode->RenderPassHandle;

    uint32 Width = 0;
    uint32 Height = 0;

    for (uint32 resourceIndex = 0; resourceIndex < inNode->Outputs.size(); ++resourceIndex)
    {
        FFrameGraphResource* Resource = inNode->Outputs[resourceIndex];
        FFrameGraphResourceInfo& Info = Resource->ResourceInfo;

        if (Resource->Type == EFrameGraphResourceType::Buffer || Resource->Type == EFrameGraphResourceType::Reference)
        {
            continue;
        }

        if (Width == 0)
        {
            Width = Info.TextureResourceInfo.Width;
        }
        else
        {
            VE_ASSERT(Width == Info.TextureResourceInfo.Width, VE_TEXT(""));
        }

        if (Height == 0)
        {
            Height = Info.TextureResourceInfo.Height;
        }
        else
        {
            VE_ASSERT(Height == Info.TextureResourceInfo.Height, VE_TEXT(""));
        }

        FFrameBufferAttachment Attachment = { };
        Attachment.Attachment = Info.TextureResourceInfo.TextureHandle;
        FrameBufferConfig.Attachments.push_back(Attachment);
    }

    for (uint32 resourceIndex = 0; resourceIndex < inNode->Inputs.size(); ++resourceIndex)
    {
        FFrameGraphResource* InputResource = inNode->Inputs[resourceIndex];

        if (InputResource->Type == EFrameGraphResourceType::Buffer || InputResource->Type == EFrameGraphResourceType::Reference)
        {
            continue;
        }

        FFrameGraphResource* Resource = inFrameGraph->GetResource(InputResource->Name);
        FFrameGraphResourceInfo& Info = Resource->ResourceInfo;

        InputResource->ResourceInfo.TextureResourceInfo.TextureHandle = Info.TextureResourceInfo.TextureHandle;

        if (Width == 0)
        {
            Width = Info.TextureResourceInfo.Width;
        }
        else
        {
            VE_ASSERT(Width == Info.TextureResourceInfo.Width, VE_TEXT(""));
        }

        if (Height == 0)
        {
            Height = Info.TextureResourceInfo.Height;
        }
        else
        {
            VE_ASSERT(Height == Info.TextureResourceInfo.Height, VE_TEXT(""));
        }

        if (InputResource->Type == EFrameGraphResourceType::Texture)
        {
            continue;
        }

        FFrameBufferAttachment Attachment = { };
        Attachment.Attachment = Info.TextureResourceInfo.TextureHandle;
        FrameBufferConfig.Attachments.push_back(Attachment);
    }

    FrameBufferConfig.Resolution.Width = Width;
    FrameBufferConfig.Resolution.Height = Height;

    inNode->FrameBufferHandle = Renderer::Get().GetRenderInterface().Get()->CreateFrameBuffer(FrameBufferConfig);
}
