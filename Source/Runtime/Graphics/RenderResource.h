/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Core/Misc/Interface.h>
#include "RenderResourceGenerics.h"

/**
* Generic interface for a resource used in the render system 
*/
class VRIXIC_API RenderResource : public Interface
{
public:
    /**
    * @returns EResourceType the type of resource this object is 
    */
    virtual inline EResourceType GetResourceType() const = 0;

public:
    uint32 ResourcePoolIndex = UINT32_MAX;
};