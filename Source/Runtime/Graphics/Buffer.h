/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include "RenderResource.h"

/**
* Holds the desription of a buffer of any type
*/
struct VRIXIC_API BufferDescriptor
{
public:
    // The size of the buffer in bytes.
    uint64 Size;

    // this are flags that specify the usage of the buffer BufferUsageFlags::Index, etc..
    uint32 UsageFlags;

    // Binding flags for the buffer 
    uint32 BindFlags;

    // Initial data which the buffer will be initialized with (raw pointer to the data)
    const void* InitialData;

public:
    BufferDescriptor()
    {
        Size = 0;
        UsageFlags = 0;
        BindFlags = 0;
        InitialData = nullptr;
    }
};

/**
* A buffer interface, but not really since it does have some definition make it an abstract class, so I is stripped off of the name 
*/
class VRIXIC_API Buffer : RenderResource
{
protected:
    BufferDescriptor BufferDesc;

public:
    /**
    * The usage flags of this buffer 
    */
    inline uint32 GetUsageFlags() const
    {
        return BufferDesc.UsageFlags;
    }

    /**
    * The bind flags of this buffer
    */
    inline uint32 GetBindFlags() const
    {
        return BufferDesc.BindFlags;
    }

    /**
    * @returns BufferDescriptor a info struct of the buffer description, can contain things like size and other flags
    */
    virtual BufferDescriptor GetDescriptor() const
    {
        return BufferDesc;
    }
};
