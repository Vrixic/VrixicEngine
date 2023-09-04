/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Core/Core.h>
#include <Misc/Defines/GenericDefines.h>
#include <Runtime/Graphics/Buffer.h>
#include <Runtime/Graphics/Format.h>
#include <Runtime/Graphics/RenderPass.h>
#include <Runtime/Graphics/RenderPassGenerics.h>
#include <Runtime/Graphics/Texture.h>

class ICommandBuffer;

enum class EFrameGraphResourceType
{
    Invalid = -1,

    Buffer,
    Texture,
    Attachment,
    Reference
};

typedef uint32 FrameGraphHandle;

struct VRIXIC_API FFrameGraphNodeHandle
{
    FrameGraphHandle Handle;
};

struct VRIXIC_API FFrameGraphResourceHandle
{
    FrameGraphHandle Handle;
};

/**
* Contains resource information (Buffer or Texture)
* @note Do not inherit from this structure as it uses unions 
*/
struct VRIXIC_API FFrameGraphResourceInfo
{
public:
    bool bIsExternalResource = false;

    union
    {
        struct
        {
            /** Size of the buffer */
            uint64 Size = 0;

            /** The buffer usage flags */
            uint32 UsageFlags = 0;

            /** The buffer handle (should get replaced to a uint32 and not a raw pointer :{ ) */
            Buffer* BufferHandle = nullptr;
        } BufferResourceInfo;

        struct 
        {
            /** Extext of the texture */
            uint32 Width = 0;
            uint32 Height = 0;
            uint32 Depth = 0;

            /** Format of the texture layout */
            EPixelFormat Format = EPixelFormat::Undefined;

            /** Defines how the texture will be used */
            uint32 UsageFlags = 0;

            /** Relevant for attachment outputs as it allows us to control if we want to discard previous content (first time usage [ex: depth]) or load previous data from last pass */
            EAttachmentLoadOp LoadOp = EAttachmentLoadOp::Undefined;

            /** The handle to the texture (should get replaced to a uint32 and not a raw pointer :{ ) */
            TextureResource* TextureHandle = nullptr;
        } TextureResourceInfo = { };
    };

public:
    FFrameGraphResourceInfo() { } 
};

/**
* Defines an input or output of a node (Used to define edges b/w graph nodes)
* 
* @note an input can either be used as a texture or an attachment 
* @note an output always implies that it is an attachment with a load operation
*/
struct VRIXIC_API FFrameGraphResource
{
public:
    /** Defines the type of resource in use (Buffer or Texture..) */
    EFrameGraphResourceType      Type = EFrameGraphResourceType::Invalid;

    /** Provides information of the resource in use based in the type */
    FFrameGraphResourceInfo      ResourceInfo = { };

    /** Reference to the node that outputs the resource. (Determines the edges in the graph) */
    struct FFrameGraphNode*        Producer = nullptr;
    //FrameGraphNodeHandle        Producer = { 0 };

    /** Stores the parent resource */
    struct FFrameGraphResource*    OutputHandle = nullptr;
    //FrameGraphResourceHandle    OutputHandle = { 0 };

    /** Used for keeping reference counts for aliasing technique (allows usage of multiple resources to share the same memory) */
    int32                       ReferenceCount = 0;

    /** Name of the resource */
    std::string                 Name;

public:
    FFrameGraphResource() { }
};

struct VRIXIC_API FFrameGraphRenderPass
{
public:
    virtual void Render(ICommandBuffer* inCommandBuffer) { };

};

struct VRIXIC_API FFrameGraphNode
{
public:
    int32 ReferenceCount = 0;

    /** The handle to the renderpass (should get replaced to a uint32 and not a raw pointer :{ ) */
    IRenderPass* RenderPassHandle = nullptr;

    /** The handle to the Frame Buffer (should get replaced to a uint32 and not a raw pointer :{ ) */
    IFrameBuffer* FrameBufferHandle = nullptr;

    FFrameGraphRenderPass* GraphRenderPass = nullptr;

    /** Lists of inputs for this node */
    std::vector<FFrameGraphResource*> Inputs;
    //std::vector<FrameGraphResourceHandle> Inputs;

    /** List of outputs from this node */
    std::vector<FFrameGraphResource*> Outputs;
    //std::vector<FrameGraphResourceHandle> Outputs;

    /** All of the nodes this node is connected to */
    std::vector<FFrameGraphNode*> Edges;
    //std::vector<FrameGraphNodeHandle> Edges;

    bool bIsEnabled = true;

    std::string Name;

public:
    FFrameGraphNode() { } 
};
