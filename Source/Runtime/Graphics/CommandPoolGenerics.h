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
struct VRIXIC_API FCommandPoolConfig
{
public:
    /** The queue family index to use to create the command pool */
    uint32 QueueFamilyIndex;

public:
    FCommandPoolConfig()
        : QueueFamilyIndex(UINT32_MAX) { }

    FCommandPoolConfig(const FCommandPoolConfig&) = default;
    FCommandPoolConfig& operator = (const FCommandPoolConfig&) = default;
};
