#include "FrameGraphBuilder.h"
#include <Misc/Assert.h>
#include <Misc/Defines/StringDefines.h>

void FrameGraphBuilder::Init()
{
}

void FrameGraphBuilder::Shutdown()
{
    for (auto It = NodeMap.Begin(); It != NodeMap.End(); ++It)
    {
        delete It->second;
    }

    for (auto It = ResourceMap.Begin(); It != ResourceMap.End(); ++It)
    {
        delete It->second;
    }

    for (auto It = RenderPassMap.Begin(); It != RenderPassMap.End(); ++It)
    {
        delete It->second;
    }
}

void FrameGraphBuilder::RegisterRenderPass(std::string& inName, FFrameGraphRenderPass* inRenderPass)
{
    {
        auto It = RenderPassMap.Find(inName);
        if (It != RenderPassMap.End())
        {
            return;
        }
    }

    RenderPassMap.Add(inName, inRenderPass);

    auto It = NodeMap.Find(inName);
    VE_ASSERT(It != NodeMap.End(), VE_TEXT("[FrameGraphBuilder]: error..."));

    FFrameGraphNode* Node = It->second;
    Node->GraphRenderPass = inRenderPass;
}

FFrameGraphResource* FrameGraphBuilder::CreateNodeOutput(const FFrameGraphResourceOutputCreation& inCreation, FFrameGraphNode* inProducer)
{
    FFrameGraphResource* Resource = new FFrameGraphResource();
    Resource->Type = inCreation.Type;
    Resource->Name = inCreation.Name;

    if (inCreation.Type != EFrameGraphResourceType::Reference)
    {
        Resource->ResourceInfo = inCreation.ResourceInfo;
        Resource->OutputHandle = Resource;
        Resource->Producer = inProducer;
        Resource->ReferenceCount = 0;

        ResourceMap.Add(Resource->Name, Resource);
    }

    return Resource;
}

FFrameGraphResource* FrameGraphBuilder::CreateNodeInput(const FFrameGraphResourceInputCreation& inCreation)
{
    FFrameGraphResource* Resource = new FFrameGraphResource();

    Resource->ResourceInfo = { };
    Resource->Producer = nullptr;
    Resource->OutputHandle = nullptr;
    Resource->Type = inCreation.Type;
    Resource->Name = inCreation.Name;
    Resource->ReferenceCount = 0;

    return Resource;
}

FFrameGraphNode* FrameGraphBuilder::CreateNode(const FFrameGraphNodeCreation& inCreation)
{
    FFrameGraphNode* Node = new FFrameGraphNode();
    Node->Name = inCreation.Name;
    Node->bIsEnabled = inCreation.bIsEnabled;
    Node->Inputs.resize(inCreation.Inputs.size());
    Node->Outputs.resize(inCreation.Outputs.size());
    Node->Edges.resize(inCreation.Outputs.size());
    Node->FrameBufferHandle = nullptr;
    Node->RenderPassHandle = nullptr;

    NodeMap.Add(inCreation.Name, Node);

    // Firstly create the outputs, then we match the input resource with the right handles
    for (uint64 i = 0; i < inCreation.Outputs.size(); ++i)
    {
        const FFrameGraphResourceOutputCreation& OutputCreation = inCreation.Outputs[i];
        FFrameGraphResource* Output = CreateNodeOutput(OutputCreation, Node);
        Node->Outputs[i] = Output;
    }

    for (uint64 i = 0; i < inCreation.Inputs.size(); ++i)
    {
        const FFrameGraphResourceInputCreation& InputCreation = inCreation.Inputs[i];
        FFrameGraphResource* Input = CreateNodeInput(InputCreation);
        Node->Inputs[i] = Input;
    }

    return Node;
}

FFrameGraphNode* FrameGraphBuilder::GetNode(std::string& inNodeName)
{
    auto It = NodeMap.Find(inNodeName);
    if (It == NodeMap.End())
    {
        return nullptr;
    }

    return It->second;
}

FFrameGraphNode* FrameGraphBuilder::AccessNode(FFrameGraphNodeHandle inNodeHandle)
{
    return nullptr;
}

FFrameGraphResource* FrameGraphBuilder::GetResource(std::string& inResourceName)
{
    auto It = ResourceMap.Find(inResourceName);
    if (It == ResourceMap.End())
    {
        return nullptr;
    }

    return It->second;
}

FFrameGraphResource* FrameGraphBuilder::AccessResource(FFrameGraphResourceHandle inResourceHandle)
{
    return nullptr;
}
