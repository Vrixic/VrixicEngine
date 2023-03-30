/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Core/Core.h>

/**
* All of the shader types
*/
enum class EShaderType
{
    Undefined,          // unknown shader (could cause weird errors)
    Vertex,             // vertex shader
    TessControl,        // tessellation control shader, aka. Hull Shader
    TessEvaluation,     // tessellation evaluation shader aka. Domain Shader
    Geometry,           // geometry shader
    Fragment,           // fragment shader aka. Pixel Shader
    Compute,            // compute shader 
};

/**
* All shader stages flags 
*/
struct VRIXIC_API ShaderStageFlags
{
#define BIT(x) (1 << x)

    // Stages specified in order (same as render pipeline)
    enum
    {
        // vertex shader stage 
        VertexStage                     = BIT(0),

        // Hull Shader - tessellation-control stage
        TessControlStage                = BIT(1),

        // Domain Shader - tessellation-evaluation stage
        TessEvaluationStage             = BIT(2),

        // geometry shader satge 
        GeometryStage                   = BIT(3),

        // pixel - fragment shader stage
        FragmentStage                   = BIT(4),

        // computer shader stage 
        ComputeStage                    = BIT(5),

        // Defines all tessellation stages
        TessellationStage               = (TessControlStage | TessEvaluationStage),

        // defines all graphics stages
        GraphicStages                   = (VertexStage | FragmentStage | GeometryStage | TessellationStage),

        // Defins all stages
        AllStage                        = (GraphicStages | ComputeStage),

        // defines default shader stages which are vertex and fragment 
        DefaultStages                   = (VertexStage | FragmentStage)
    };
};