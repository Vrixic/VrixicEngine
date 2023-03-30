/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Core/Misc/Interface.h>

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
};

/**
* A buffer interface, but not really since it does have some definition make it an abstract class, so I is stripped off of the name 
*/
class VRIXIC_API Buffer : Interface
{
private:
    uint32 UsageFlags;
public:

    /**
    * The buffer usage flags of this buffer 
    */
    inline uint32 GetUsageFlags() const
    {
        return UsageFlags;
    }

    /**
    * @returns BufferDescriptor a info struct of the buffer description, can contain things like size and other flags
    */
    virtual BufferDescriptor GetDescriptor() const = 0;
};
