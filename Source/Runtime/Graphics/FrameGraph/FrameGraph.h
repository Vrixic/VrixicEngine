#pragma once

#include "FrameGraphBuilder.h"

#include <Misc/Assert.h>
#include <Misc/Logging/Log.h>
#include <Misc/Defines/StringDefines.h>

class VRIXIC_API FrameGraph
{
public:
    void Init(FrameGraphBuilder* inBuilder);
    void Shutdown();

    void Parse(const std::string inFilePath);

    void Reset();
    void EnableRenderPass(std::string inRenderPassName);
    void DisableRenderPass(std::string inRenderPassName);

    void Compile();

    FFrameGraphNode* GetNode(std::string& inName);
    FFrameGraphResource* GetResource(std::string& inName);

    inline const FrameGraphBuilder* GetBuilder()
    {
        return GraphBuilder;
    }

private:
    static void ComputeEdges(FrameGraph* inFrameGraph, FFrameGraphNode* inNode, uint32 inNodeIndex);

    static void CreateRenderPass(FrameGraph* inFrameGraph, FFrameGraphNode* inNode);

    static void CreateFrameBuffer(FrameGraph* inFrameGraph, FFrameGraphNode* inNode);

private:

    /** Nodes stored in topological order */
    std::vector<FFrameGraphNode*> Nodes;

    FrameGraphBuilder* GraphBuilder;

    std::string Name;
};