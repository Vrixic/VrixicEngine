/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Core/Misc/Interface.h>
#include "PipelineGenerics.h"
#include "PipelineLayout.h"
#include "Sampler.h"
#include "Texture.h"

class IDescriptorSets;

/**
* A configuration struct used to create descriptor sets 
*/
struct VRIXIC_API FDescriptorSetsConfig
{
public:
    FDescriptorSetsConfig() : NumSets(1), PipelineLayoutPtr(nullptr) { }

    ~FDescriptorSetsConfig() { }

public:
    /** Number of sets to create */
    uint32 NumSets;

    /** Pointer to the pipeline layout the descriptor set will use to be created */
    PipelineLayout* PipelineLayoutPtr;
};

/**
* Info struct that helps with binding descriptor sets
*/
struct VRIXIC_API FDescriptorSetsBindInfo
{
public:
    FDescriptorSetsBindInfo() 
        : PipelineLayoutPtr(nullptr), PipelineBindPoint(EPipelineBindPoint::Graphics), DescriptorSets(nullptr), NumSets(1) { }

    ~FDescriptorSetsBindInfo() { }

public:
    /** The pointer to the pipeline layout that was used to program the bindings for the descriptor sets */
    PipelineLayout* PipelineLayoutPtr;

    /** Pointer to the descriptor sets that will get bound */
    IDescriptorSets* DescriptorSets;

    /** Indicates which bind point the descriptor set(s) will get bound to */
    EPipelineBindPoint PipelineBindPoint;

    /** Number of descriptor sets to bind */
    uint32 NumSets;
};

/**
* Info struct that helps with the linking(update) buffers or textures to descriptor sets 
*/
struct VRIXIC_API FDescriptorSetsLinkInfo
{
public:
    FDescriptorSetsLinkInfo() : TextureSampler(nullptr), BindingStart(0), DescriptorCount(1), ArrayElementStart(0) { }

    ~FDescriptorSetsLinkInfo() { }

public:
    union VRIXIC_API UResourceHandle {
        /** The buffer to bind to the descriptor set(s) */
        Buffer* BufferHandle;

        /** The texture to bind to the descriptor set(s) */
        Texture* TextureHandle;

        UResourceHandle() : BufferHandle(nullptr) { } 
    };

    /** Resource handle is the handle to the resource to be updated to the descriptor set */
    UResourceHandle ResourceHandle;

    /** The texture sampler used for texture resources */
    Sampler* TextureSampler;

    /** 
    * The Index specifing the start of the binding 
    * 
    * For Example: 
    *   writing 4 descriptor sets having 3 storage buffer and one uniform buffer
    *   First write Binding Start = 0, as we want to update the storage buffers
    *   But Binding Start = 3 means we will start at the uniform buffer and update the uniform buffer
    */
    uint32 BindingStart;

    /**
    * The count of descriptors to update 
    * 
    * For Example: From above example
    *   4 sets, 3 storage buffers and one uniform
    *   so first write will be 3 and second will be 1, etc...
    */
    uint32 DescriptorCount;

    /**
    * If we are updating an array, this indicates the start index of the element in the array that will be updated
    * 
    * For Example: We have 100 textures but only want to update the 50th
    *   so ArrayElementStart will be =  to 50, 
    *   Then if we only want to update 2 textures,
    *   DescriptorCount will be 2 
    */
    uint32 ArrayElementStart;
};

/**
* A generic descriptor set -> These are used to specify the resources to be bound to a shader 
* Can contain one or more descriptor sets 
*/
class VRIXIC_API IDescriptorSets : public Interface
{
public:
    /**
    * Links the specified descriptor set to a buffer resource
    * 
    * @param inIndex the descriptor set to update
    * @param inDescriptorSetsLinkInfo information used to link the buffer to the descriptor set 
    */
    virtual void LinkToBuffer(uint32 inIndex, const FDescriptorSetsLinkInfo& inDescriptorSetsLinkInfo) = 0;

    /**
    * Links the specified descriptor set to a texture resource
    *
    * @param inIndex the descriptor set to update
    * @param inDescriptorSetsLinkInfo information used to link the texture to the descriptor set
    */
    virtual void LinkToTexture(uint32 inIndex, const FDescriptorSetsLinkInfo& inDescriptorSetsLinkInfo) = 0;

public:
    /**
    * @returns uint32 the number of descriptor sets contained in this object
    */
    inline uint32 GetNumSets() const
    {
        return NumSets;
    }

protected:
    /** Number of descriptor sets contained in this object */
    uint32 NumSets;
};
