/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Core/Core.h>
#include "Extents.h"
#include <Misc/Defines/GenericDefines.h>

/**
* Helper struct used for Swap Chain configuration 
*/
struct VRIXIC_API SwapChainConfig
{
    /**
    * The screen resolution in pixels
    * 
    * @remarks the renderer could invalidate the resolution in here
    */
    Extent2D ScreenResolution;

    /**
    * The number of bits for each pixel in the color buffer, usually 32 
    * 
    * @remarks the renderer could override this value 
    */
    uint32 ColorBits;

    /**
    * The number of bits used for the depth value in the depth buffer, usually 24 
    * 
    * @remarks the rendere could override this value
    */
    uint32 DepthBits;

    /**
    * The number of bits used for the stencil value in the stencil buffer, usually 8, zero to disable 
    * 
    * @remarks 
    */
    uint32 StencilBits;

    /**
    * The number of swap buffers in use by the swap chain, usally two for double-buffer technique (back and front)
    */
    uint32 NumSwapBuffers;

    /**
    * Used to set if VSync should be enabled or not 
    */
    bool bEnableVSync;

public:
    SwapChainConfig();
    ~SwapChainConfig();

    inline static SwapChainConfig CreateDefaultConfig();

private:
    inline void SetDefaultConfig();
};

inline SwapChainConfig::SwapChainConfig()
{
    SetDefaultConfig();
}

inline SwapChainConfig::~SwapChainConfig() { }

inline SwapChainConfig SwapChainConfig::CreateDefaultConfig()
{
    SwapChainConfig Desc = { };
    Desc.SetDefaultConfig();

    return Desc;
}

inline void SwapChainConfig::SetDefaultConfig()
{
    ScreenResolution = Extent2D(1280, 720);
    ColorBits = 32;
    DepthBits = 24;
    StencilBits = 8;
    NumSwapBuffers = 2;
    bEnableVSync = false;
}