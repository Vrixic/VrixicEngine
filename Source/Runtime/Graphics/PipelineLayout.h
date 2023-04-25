/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Core/Misc/Interface.h>
#include "RenderResourceGenerics.h"

#include <vector>

/**
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
struct VRIXIC_API FPipelineBindingDescriptor
{
public:
    /** Resource type of this binding: buffer, texture, etc... */
    EResourceType ResourceType;

    /** amount of resources getting bound ex: 5 uniform buffers */
    uint32 NumResources;

    /** Specifies an indepth overriew of where the resource type gets bound to, for ex: Vertex Buffer */
    uint32 BindFlags;

    /** Specifies the shader stages that this binding will get bound to */
    uint32 StageFlags;

    /** Specifies the binding slot / binding point for this descriptor */
    FPipelineBindingSlot BindingSlot;

public:
    FPipelineBindingDescriptor()
        : ResourceType(EResourceType::Undefined), NumResources(0), BindFlags(0), StageFlags(0) { }

    FPipelineBindingDescriptor(const FPipelineBindingDescriptor&) = default;
};

/**
* Defines a pipeline layout 
*/
struct VRIXIC_API FPipelineLayoutConfig
{
public:
    /** Consists of all the bindings */
    std::vector<FPipelineBindingDescriptor> Bindings;
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
    PipelineLayout(const FPipelineLayoutConfig& inPipelineLayoutDescriptor)
        : Bindings (inPipelineLayoutDescriptor.Bindings) { }

public:
    /**
    * @returns uint32 number of bindings attached to this pipeline layout
    */
    inline virtual uint32 GetNumBinding() const
    {
        return (uint32)Bindings.size();
    }

    /**
    * @returns const std::vector<PipelineBindingDescriptor>& the binding that are associated with this layout
    */
    inline const std::vector<FPipelineBindingDescriptor>& GetBindings() const
    {
        return Bindings;
    }

private:
    /** All of the bindings associated with this Pipeline Layout */
    std::vector<FPipelineBindingDescriptor> Bindings;

};