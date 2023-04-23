/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Core/Core.h>
#include <Misc/Defines/GenericDefines.h>

/**
* Used to speficy an extent, mainly used for screen (Framebuffers)
*/
struct VRIXIC_API Extent2D
{
public:
    uint32 Width;
    uint32 Height;

public:
    Extent2D()
        : Width(0), Height(0) { }

    Extent2D(uint32 inWidth, uint32 inHeight)
        : Width(inWidth), Height(inHeight) { }
};

/**
* Defines an extent 3d which contains a depth with width and height 
*/
struct VRIXIC_API Extent3D
{
public:
    uint32 Width;
    uint32 Height;
    uint32 Depth;

public:
    Extent3D()
        : Width(0), Height(0), Depth(0) { }

    Extent3D(uint32 inWidth, uint32 inHeight, uint32 inDepth)
        : Width(inWidth), Height(inHeight), Depth(inDepth) { }
};

