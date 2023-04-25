/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Core/Misc/Interface.h>
#include "PipelineGenerics.h"

/**
* Pipeline state interface aka. VkPipeline for vulkan 
*/
class VRIXIC_API IPipeline : Interface
{ 
public:
    /**
    * @returns EPipelineBindPoint the bindpoint for this pipeline, ex. Graphics, Compute, etc...
    */
    inline virtual EPipelineBindPoint GetBindPoint() const = 0;
};