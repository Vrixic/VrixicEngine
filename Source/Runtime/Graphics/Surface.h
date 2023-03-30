/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Core/Misc/Interface.h>
#include "Format.h"

/**
* A surface interface which we can 
*/
class VRIXIC_API Surface : public Interface
{
    /**
    * @returns EFormat the color format of this swapchain
    */
    virtual EFormat GetColorFormat() const = 0;
};