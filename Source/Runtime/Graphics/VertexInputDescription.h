/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include "VertexInputAttribute.h"

#include <vector>

enum class EInputRate
{
   Vertex       = 0, /** input rate is vertex */
   Instance     = 1, /** input rate is instance */
};

struct VRIXIC_API FVertexInputDescription
{
public:
    /** The binding number that this structure describes */ 
    uint32 BindingNum;

    /** The byte stride(offset) between consecutive elements within the buffer */
    uint32 Stride;

    /** 
    * Input Rate - specifies whether a vertex attribute addressing a function of the vertex 
    * index or of the instance index
    */
    EInputRate InputRate;

public:
    inline FVertexInputDescription()
        : BindingNum(0), Stride(0), InputRate(EInputRate::Vertex) { }

    inline FVertexInputDescription(const FVertexInputDescription&) = default;
    inline FVertexInputDescription& operator=(const FVertexInputDescription&) = default;

    inline void AddVertexAttribute(const FVertexInputAttribute& inAttribute);

    inline const std::vector<FVertexInputAttribute>& GetVertexAttributes() const;

private:
    /** All of the attributes that define this vertex input description */
    std::vector<FVertexInputAttribute> Attributes;

};

void FVertexInputDescription::AddVertexAttribute(const FVertexInputAttribute& inAttribute)
{
    Attributes.push_back(inAttribute);
}

const std::vector<FVertexInputAttribute>& FVertexInputDescription::GetVertexAttributes() const
{
    return Attributes;
}
