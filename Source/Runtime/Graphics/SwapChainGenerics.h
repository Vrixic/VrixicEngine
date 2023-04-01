/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Core/Core.h>
#include <Misc/Defines/GenericDefines.h>
#include <Runtime/Core/Math/Vector2D.h>


struct VRIXIC_API SwapChainConfig
{
    /**
    * The screen resolution in pixels
    * 
    * @remarks the renderer could invalidate the resolution in here
    */
    Vector2D ScreenResolution;

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

    inline static SwapChainConfig& CreateDefaultDescriptor();

private:
    inline void SetDefaultDescritor();
};

inline SwapChainConfig::SwapChainConfig()
{
    SetDefaultDescritor();
}

inline SwapChainConfig::~SwapChainConfig() { }

inline SwapChainConfig& SwapChainConfig::CreateDefaultDescriptor()
{
    SwapChainConfig Desc = { 0 };
    Desc.SetDefaultDescritor();

    return Desc;
}

inline void SwapChainConfig::SetDefaultDescritor()
{
    ScreenResolution = Vector2D(1280.0f, 720.0f);
    ColorBits = 32;
    DepthBits = 24;
    StencilBits = 8;
    NumSwapBuffers = 2;
    bEnableVSync = false;
}