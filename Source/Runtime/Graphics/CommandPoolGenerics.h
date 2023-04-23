/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Core/Core.h>
#include <Misc/Defines/GenericDefines.h>

/**
* Contains configuration information for creating a Command Pool 
*/
struct VRIXIC_API CommandPoolConfig
{
public:
    /* The queue family index to use to create the command pool */
    uint32 QueueFamilyIndex;

public:
    CommandPoolConfig() = default;
    CommandPoolConfig(const CommandPoolConfig&) = default;
    CommandPoolConfig& operator = (const CommandPoolConfig&) = default;
};
