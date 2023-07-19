/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Core/Misc/Interface.h>
#include "RenderResourceGenerics.h"

#include <vector>

/**-------------------- Constants -----------------------*/

static const uint8 MAX_BINDINGS_PER_DESCRIPTOR = 16u;
static const uint8 MAX_DESCRIPTORS_PER_LAYOUT = 8u;

/**-------------------- Constants -----------------------*/

/**
*  // DEPRECATED \\
* 
* The binding slot or binding point of a resource or descriptor
*/
struct VRIXIC_API FPipelineBindingSlot
{
public:
    /** a zero based index that specifies the binding number */
    uint32 Index;

    /** a zero based index that specifies the descriptor set this binding slot belongs to */
    uint32 SetIndex;

public:
    FPipelineBindingSlot()
        : Index(UINT32_MAX), SetIndex(UINT32_MAX) { }

    FPipelineBindingSlot(const FPipelineBindingSlot&) = default;
};

/**
* Defines a layout for a single binding of a resource that can get bound to a pipeline layout
*/
struct VRIXIC_API FPipelineBinding
{
public:
    /** Resource type of this binding: buffer, texture, etc... */
    EResourceType ResourceType;

    /** Specifies an indepth overriew of where the resource type gets bound to, for ex: Vertex Buffer */
    uint32 BindFlags;

    /** Specifies the shader stages that this binding will get bound to */
    uint32 StageFlags;

    /** amount of resources getting bound ex: 5 uniform buffers */
    uint16 NumResources;

    /** Specifies the binding slot / binding point for this descriptor */
    uint16 BindingIndex;

public:
    FPipelineBinding()
        : ResourceType(EResourceType::Undefined),
          BindFlags(0), StageFlags(0), NumResources(0), BindingIndex(0) { }

    FPipelineBinding(const FPipelineBinding&) = default;
};

/**
* Defines a set of bindings for a binding point (Set)
*/
struct VRIXIC_API FPipelineBindingDescriptor
{
public:
    /** Bindings describing this descriptor */
    FPipelineBinding Bindings[MAX_BINDINGS_PER_DESCRIPTOR];

    /** Total Number of bindings */
    uint16 NumBindings;

    /** The binding point these bindings will be bound to (Aka. in vulkan. the descriptor set index) */
    uint16 SetIndex;

    /** Tells the internal renderer to use bindless descripting to create this pipeline binding descriptor */
    //bool bIsBindlessDescriptor;

public:
    FPipelineBindingDescriptor()
        : NumBindings(0), SetIndex(0)/*, bIsBindlessDescriptor(false)*/ { }

    FPipelineBindingDescriptor(const FPipelineBindingDescriptor&) = default;

    /**
    * Adds the specified binding to this descriptors set of bindings 
    * 
    * @param inPipelineBinding the pipeline binding to add 
    */
    void AddBinding(const FPipelineBinding& inPipelineBinding)
    {
        Bindings[NumBindings++] = inPipelineBinding;
    }

    /**
    * Adds the specified binding to the binding index specified
    * @note: this is an unsafe helper function that can cause pipeline bindings fragments
    *   if not used correctly, ensure you will be filling in the fragmented space caused by
    *   adding bindings non-linearly 
    *
    * @param inPipelineBinding the pipeline binding to add
    * @param inBindingIndex the index 'inPipelineBinding' should be set to
    */
    void AddBindingAt(const FPipelineBinding& inPipelineBinding, uint32 inBindingIndex)
    {
        Bindings[inBindingIndex] = inPipelineBinding;
        inBindingIndex = inBindingIndex + 1;

        NumBindings = inBindingIndex > NumBindings ? inBindingIndex : NumBindings;
    }
};

/**
* Defines a pipeline layout 
*/
struct VRIXIC_API FPipelineLayoutConfig
{
public:
    /** Consists of all sets with their bindings */
    FPipelineBindingDescriptor BindingDescriptors[MAX_DESCRIPTORS_PER_LAYOUT];

    /** Consists of all the bindings */
    //std::vector<FPipelineBindingDescriptor> Bindings;

    /** 
    * Number of sets for this pipeline layout in use 
    * @note: cannot have 2 sets whos indexes are adajacent to each other: 
        Ex: Set One Index (0) and Second Set Index (5): will not work 
        Ex: Set One Index (0) and Second Set Index (1): will work 
    */
    uint32 NumSets;

public:
    FPipelineLayoutConfig() : NumSets(0) { }

    /**
    * Adds a binding descriptor to the set index specified
    * @note: can cause set fragments 
    * 
    * @param inBindingDescriptor the pipeline binding descriptor to add
    * @param inSetIndex the index to add the descriptor to 
    */
    void AddBindingDescriptorAt(const FPipelineBindingDescriptor& inBindingDescriptor, uint32 inSetIndex)
    {
        //VE_ASSERT(inSetIndex < MAX_DESCRIPTORS_PER_LAYOUT, "[FPipelineLayoutConfig]: Cannot add pipeline bindings whos SetIndex is greater than the max sets per layout supported...");

        BindingDescriptors[inSetIndex] = inBindingDescriptor;
        inSetIndex++;
        NumSets = inSetIndex > NumSets ? inSetIndex : NumSets;
    }

    /**
    * @returns the pipeline binding descriptor at the specified index 
    */
    FPipelineBindingDescriptor& GetBindingDescriptorAt(uint32 inSetIndex)
    {
        return BindingDescriptors[inSetIndex];
    }
};

/**
* Defines a layout for a resource binding in a pipeline 
*/
class VRIXIC_API PipelineLayout : public Interface
{
public:
    /**
    * @remarks creates a pipeline layout with the descriptor passed in
    */
    PipelineLayout();

private:
    friend class VulkanRenderInterface;

    static uint8 BINDLESS_TEXTURE_DESCRIPTOR_INDEX;
    static uint8 BINDLESS_TEXTURE_BINDING_INDEX;
    static uint16 MAX_NUM_BINDLESS_RESOURCES;
};