#pragma once

#include "FrameGraphGenerics.h"

#include <Containers/Map.h>

struct VRIXIC_API FFrameGraphResourceOutputCreation 
{
    EFrameGraphResourceType Type = EFrameGraphResourceType::Invalid;
    FFrameGraphResourceInfo  ResourceInfo = { };

    std::string Name;
};

struct VRIXIC_API FFrameGraphResourceInputCreation
{
    EFrameGraphResourceType Type = EFrameGraphResourceType::Invalid;
    FFrameGraphResourceInfo  ResourceInfo = { };

    std::string Name;
};

struct VRIXIC_API FFrameGraphNodeCreation
{
    std::vector<FFrameGraphResourceInputCreation>  Inputs;
    std::vector<FFrameGraphResourceOutputCreation> Outputs;

    bool bIsEnabled = true;

    std::string Name;
};

class VRIXIC_API FrameGraphBuilder
{
    friend class FrameGraph;
public:
    void Init();

    void Shutdown();

    void RegisterRenderPass(std::string& inName, FFrameGraphRenderPass* inRenderPass);

    FFrameGraphResource* CreateNodeOutput(const FFrameGraphResourceOutputCreation& inCreation, FFrameGraphNode* inProducer);
    FFrameGraphResource* CreateNodeInput(const FFrameGraphResourceInputCreation& inCreation);
    FFrameGraphNode* CreateNode(const FFrameGraphNodeCreation& inCreation);

    FFrameGraphNode* GetNode(std::string& inNodeName);
    FFrameGraphNode* AccessNode(FFrameGraphNodeHandle inNodeHandle);

    FFrameGraphResource* GetResource(std::string& inResourceName);
    FFrameGraphResource* AccessResource(FFrameGraphResourceHandle inResourceHandle);

private:
    TMap<std::string, FFrameGraphRenderPass*> RenderPassMap;
    TMap<std::string, FFrameGraphResource*> ResourceMap;
    TMap<std::string, FFrameGraphNode*> NodeMap;
};

