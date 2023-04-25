/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Core/Misc/Interface.h>

/**
* Sempahore creation configuration
*/
struct VRIXIC_API FSemaphoreConfig
{
public:
    /** Number of semaphores to create */ 
    uint32 NumSemaphores;

public:
    FSemaphoreConfig(uint32 inNumSemaphores)
        : NumSemaphores(inNumSemaphores) { }
};

/**
* This is a generic semaphore that can be used for GPU synchronization for rendering and queue submitting
* 
* Can have more than one semaphore (more like a semaphore array (ex: signal semaphores))
*/
class VRIXIC_API ISemaphore : public Interface
{

};