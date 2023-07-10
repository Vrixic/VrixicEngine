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
struct VRIXIC_API FExtent2D
{
public:
    uint32 Width;
    uint32 Height;

public:
    FExtent2D()
        : Width(0), Height(0) { }

    FExtent2D(uint32 inWidth, uint32 inHeight)
        : Width(inWidth), Height(inHeight) { }
};

/**
* Defines an extent 3d which contains a depth with width and height 
*/
struct VRIXIC_API FExtent3D
{
public:
    uint32 Width;
    uint32 Height;
    uint32 Depth;

public:
    FExtent3D()
        : Width(0), Height(0), Depth(0) { }

    FExtent3D(uint32 inWidth, uint32 inHeight, uint32 inDepth)
        : Width(inWidth), Height(inHeight), Depth(inDepth) { }
};

/**
* Defines a 3D offset 
*/
struct VRIXIC_API FOffset3D
{
public:
    int32 X;
    int32 Y;
    int32 Z;

public:
    FOffset3D()
        : X(0), Y(0), Z(0) { }

    FOffset3D(int32 inX, int32 inY, int32 inZ)
        : X(inX), Y(inY), Z(inZ) { }
};

struct VRIXIC_API FRect2D
{
public:
    uint32 OffsetX;
    uint32 OffsetY;
    uint32 Width;
    uint32 Height;

public:
    FRect2D()
        : OffsetX(0), OffsetY(0), Width(0), Height(0) { }
};

