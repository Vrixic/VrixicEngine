/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once

enum class EFormat
{
    Undefined = 0,          // Color format not known

    /* Alpha channel */
    A8UNorm,                // alpha 8-bit normalized unsigned integer 

    /* Red channel */
    R8UNorm,                // red 8-bit normalized unsigned integer 
    R8SNorm,                // red 8-bit normalized signed integer 
    R8UInt,                 // red 8-bit unsigned integer 
    R8SInt,                 // red 8-bit signed integer 
    R8SRGB,                 // red 8-bit unsigned normalized with sRGB nonlinear encoding  
                               
    R16UNorm,               // red 16-bit normalized unsigned interger 
    R16SNorm,               // red 16-bit normalized signed interger 
    R16UInt,                // red 16-bit unsigned interger 
    R16SInt,                // red 16-bit signed interger 
    R16Float,               // red 16-bit floating point 
                               
    R32UInt,                // red 32-bit unsigned interger 
    R32SInt,                // red 32-bit signed interger 
    R32Float,               // red 32-bit floating point 
                               
    R64Float,               // red 64-bit floating point component

    /* red | green channels */
    RG8UNorm,               // red | green 8-bit normalized unsigned integer
    RG8SNorm,               // red | green 8-bit normalized signed integer
    RG8UInt,                // red | green 8-bit unsigned integer
    RG8SInt,                // red | green 8-bit signed integer
                                   
    RG16UNorm,              // red | green 16-bit normalized unsigned interger
    RG16SNorm,              // red | green 16-bit normalized signed interger
    RG16UInt,               // red | green 16-bit unsigned interger
    RG16SInt,               // red | green 16-bit signed interger
    RG16Float,              // red | green 16-bit floating point
                                   
    RG32UInt,               // red | green 32-bit unsigned interger
    RG32SInt,               // red | green 32-bit signed interger
    RG32Float,              // red | green 32-bit floating point
                                   
    RG64Float,              // red | green 64-bit floating point

    /* red | green | blue channels */
    RGB8UNorm,              // red | green | blue 8-bit normalized unsigned integer 
    RGB8UNorm_sRGB,         // red | green | blue 8-bit normalized unsigned integer components in non-linear sRGB color space. 
    RGB8SNorm,              // red | green | blue 8-bit normalized signed integer 
    RGB8UInt,               // red | green | blue 8-bit unsigned integer 
    RGB8SInt,               // red | green | blue 8-bit signed integer 
                               
    RGB16UNorm,             // red | green | blue 16-bit normalized unsigned interger 
    RGB16SNorm,             // red | green | blue 16-bit normalized signed interger 
    RGB16UInt,              // red | green | blue 16-bit unsigned interger 
    RGB16SInt,              // red | green | blue 16-bit signed interger 
    RGB16Float,             // red | green | blue 16-bit floating point 
                               
    RGB32UInt,              // red | green | blue 32-bit unsigned interger 
    RGB32SInt,              // red | green | blue 32-bit signed interger 
    RGB32Float,             // red | green | blue 32-bit floating point 
                               
    RGB64Float,             // red | green | blue 64-bit floating point

    /* red | green | blue | alpha channels */
    RGBA8UNorm,             // red | green | blue | alpha 8-bit normalized unsigned integer
    RGBA8UNorm_sRGB,        // red | green | blue | alpha 8-bit normalized unsigned integer components in non-linear sRGB color space.
    RGBA8SNorm,             // red | green | blue | alpha 8-bit normalized signed integer
    RGBA8UInt,              // red | green | blue | alpha 8-bit unsigned integer
    RGBA8SInt,              // red | green | blue | alpha 8-bit signed integer
                               
    RGBA16UNorm,            // red | green | blue | alpha 16-bit normalized unsigned interger
    RGBA16SNorm,            // red | green | blue | alpha 16-bit normalized signed interger
    RGBA16UInt,             // red | green | blue | alpha 16-bit unsigned interger
    RGBA16SInt,             // red | green | blue | alpha 16-bit signed interger
    RGBA16Float,            // red | green | blue | alpha 16-bit floating point
                               
    RGBA32UInt,             // red | green | blue | alpha 32-bit unsigned interger
    RGBA32SInt,             // red | green | blue | alpha 32-bit signed interger
    RGBA32Float,            // red | green | blue | alpha 32-bit floating point
                               
    RGBA64Float,            // red | green | blue | alpha 64-bit floating point

    /* BGRA color formats */
    BGRA8UNorm,             // blue | green | red | alpha 8-bit normalized unsigned integer
    BGRA8UNorm_sRGB,        // blue | green | red | alpha 8-bit normalized unsigned integer components in non-linear sRGB color space.
    BGRA8SNorm,             // blue | green | red | alpha 8-bit normalized signed integer
    BGRA8UInt,              // blue | green | red | alpha 8-bit unsigned integer
    BGRA8SInt,              // blue | green | red | alpha 8-bit signed integer

    /* Depth-stencil formats */
    D16UNorm,               // depth 16-bit normalized unsigned integer 
    D24UNormS8UInt,         // depth 24-bit normalized unsigned integer component, and 8-bit unsigned integer stencil 
    D32Float,               // depth 32-bit floating point 
    D32FloatS8X24UInt,      // depth 32-bit floating point component, and 8-bit unsigned integer stencil components (where the remaining 24 bits are unused).
    S8UInt                  // Stencil only format: 8-bit unsigned integer stencil

};