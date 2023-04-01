/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Core/Misc/Interface.h>
#include "RenderResourceGenerics.h"

#include <vector>

/**
* Defines a layout for a single binding of a resource that can get bound to a pipeline layout
*/
struct VRIXIC_API PipelineBindingDescriptor
{
public:
    PipelineBindingDescriptor() = default;
    PipelineBindingDescriptor(const PipelineBindingDescriptor&) = default;

public:
    /** Resource type of this binding: buffer, texture, etc... */
    EResourceType ResourceType;

    /** The number/slot this descritor gets bound to */
    uint32 BindingNum;

    /** amount of resources getting bound ex: 5 uniform buffers */
    uint32 NumResources;

    /** Specifies an indepth overriew of where the resource type gets bound to, for ex: Vertex Buffer */
    uint32 BindFlags;

    /** Specifies the shader stages that this binding will get bound to */
    uint32 StageFlags;
};

/**
* Defines a pipeline layout 
*/
struct VRIXIC_API PipelineLayoutConfig
{
public:
    /** Consists of all the bindings */
    std::vector<PipelineBindingDescriptor> Bindings;
};

/**
* Defines a layout for a resource binding in a pipeline 
*/
class VRIXIC_API PipelineLayout : public Interface
{
private:
    /** All of the bindings associated with this Pipeline Layout */
    std::vector<PipelineBindingDescriptor> Bindings;

public:
    /**
    * @remarks creates a pipeline layout with the descriptor passed in
    */
    PipelineLayout(const PipelineLayoutConfig& inPipelineLayoutDescriptor)
        : Bindings (inPipelineLayoutDescriptor.Bindings) { }

public:
    /**
    * @returns uint32 number of bindings attached to this pipeline layout
    */
    inline virtual uint32 GetNumBinding() const
    {
        return Bindings.size();
    }

    /**
    * @returns const std::vector<PipelineBindingDescriptor>& the binding that are associated with this layout
    */
    inline const std::vector<PipelineBindingDescriptor>& GetBindings() const
    {
        return Bindings;
    }
};