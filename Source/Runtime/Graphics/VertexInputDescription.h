/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include "VertexInputAttribute.h"

#include <vector>

enum class EInputRate
{
   Vertex       = 0, // input rate is vertex 
   Instance     = 1, // input rate is instance 
};

struct VRIXIC_API VertexInputDescription
{
private:
    // All of the attributes that define this vertex input description
    std::vector<VertexInputAttribute> Attributes;

public:
    // The binding number that this structure describes
    uint32 BindingNum;

    // The byte stride(offset) between consecutive elements within the buffer
    uint32 Stride;

    // Input Rate - specifies whether a vertex attribute addressing a function of the vertex
    // index or of the instance index
    EInputRate InputRate;

public:
    VertexInputDescription() = default;
    VertexInputDescription(const VertexInputDescription&) = default;
    VertexInputDescription& operator=(const VertexInputDescription&) = default;

    void AddVertexAttribute(const VertexInputAttribute& inAttribute);

    const std::vector<VertexInputAttribute>& GetVertexAttributes() const;
};

void VertexInputDescription::AddVertexAttribute(const VertexInputAttribute& inAttribute)
{
    Attributes.push_back(inAttribute);
}

const std::vector<VertexInputAttribute>& VertexInputDescription::GetVertexAttributes() const
{
    return Attributes;
}
