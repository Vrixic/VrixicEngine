/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include "BufferGenerics.h"
#include "RenderResource.h"

/**
* Holds the description of a buffer of any type
*/
struct VRIXIC_API FBufferConfig
{
public:
    /** The size of the buffer in bytes. */
    uint64 Size;

    /** this are flags that specify the usage of the buffer BufferUsageFlags::Index, etc.. */
    EBufferUsageFlags UsageFlags;

    /** this flags are for the memory that the buffer will occupy */
    uint32 MemoryFlags;

    /** Initial data which the buffer will be initialized with (raw pointer to the data) */
    const void* InitialData;

public:
    FBufferConfig()
    {
        Size = 0;
        MemoryFlags = 0;
        InitialData = nullptr;
        UsageFlags = (EBufferUsageFlags)-1;
    }
};

/**
* A buffer interface, but not really since it does have some definition make it an abstract class, so I is stripped off of the name 
*/
class VRIXIC_API Buffer : RenderResource
{
public:
    /**
    * The usage flags of this buffer 
    */
    inline EBufferUsageFlags GetUsageFlags() const
    {
        return BufferConfiguration.UsageFlags;
    }

    /**
    * @returns BufferConfig& a info struct of the buffer configuration, can contain things like size and other flags
    */
    virtual const FBufferConfig& GetBufferConfig() const
    {
        return BufferConfiguration;
    }

    /**
    * @returns EResourceType the resource type of this object 
    */
    virtual inline EResourceType GetResourceType() const override final;

protected:
    FBufferConfig BufferConfiguration;
};
