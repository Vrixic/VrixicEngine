/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Core/Core.h>
#include "Format.h"
#include <Misc/Defines/GenericDefines.h>

#include <string>

/**
* Defines whether the shader is a string or a file name or path
*/
enum class EShaderSourceType
{
    String,         // It is a string containing the shader source code
    Filename,       // it is a file name 
    Filepath,       // it is a file path to the shader 
};

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

struct VRIXIC_API ShaderCompileFlags
{
#define BIT(x) (1 << x)
    enum
    {
        InvertY         = BIT(0) // vulkan specific flag that just inverts vulkans NDCs y coord
    };
};

struct VRIXIC_API ShaderConfig
{
public:
    // Defines the type of shader it is 
    EShaderType Type;

    // The souce code of the shader as a string 
    std::string SourceCode;

    // defines the type of source it is, is it a string, filename, or a full file path 
    EShaderSourceType SourceType;

    // the entrypoint function name 
    std::string EntryPoint;

    // Shader compilation flags 
    ShaderCompileFlags CompileFlags;
};
