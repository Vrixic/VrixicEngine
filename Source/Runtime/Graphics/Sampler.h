/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include "RenderResource.h"

class VRIXIC_API Sampler : public RenderResource
{
    /**
    * @returns EResourceType the type of resource this object is
    * @remarks this cannot be inherited anymore as there is no need 
    */
    virtual inline EResourceType GetResourceType() const override final;
};