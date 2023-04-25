/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Misc/Defines/GenericDefines.h>
#include <Misc/Profiling/Profiler.h>

#if  _DEBUG
#define VE_PROFILE_MEMORY(...) VE_PROFILE_FUNCTION(__VA_ARGS__)
#else
#define VE_PROFILE_MEMORY(...) 
#endif //  _DEBUG

#define VE_PROFILE_MEMORY_HEAP(...) VE_PROFILE_MEMORY(__VA_ARGS__)

#define VE_PROFILE_MEMORY_MANAGER(...) VE_PROFILE_MEMORY(__VA_ARGS__) 

#define VE_PROFILE_MEMORY_ALLOCATERS(...) VE_PROFILE_MEMORY(__VA_ARGS__)
