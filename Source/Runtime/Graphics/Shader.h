/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include "ShaderGenerics.h"
#include <Core/Misc/Interface.h>

/**
* Defines a generic shader 
*/
class VRIXIC_API Shader : public Interface
{
public:
    /**
    * @returns EShaderType the shader type for this shader 
    */
    inline EShaderType GetShaderType() const
    {
        return ShaderType;
    }

protected:
    EShaderType ShaderType;

    /** The path to the shader if available */
    std::string Path;
};
