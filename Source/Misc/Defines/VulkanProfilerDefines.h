/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Misc/Defines/GenericDefines.h>
#include <Misc/Profiling/Profiler.h>

#define VE_PROFILE_VULKAN 1

#ifdef VE_PROFILE_VULKAN

#define VE_PROFILE_VULKAN_FUNCTION(...) \
if (VE_PROFILE_VULKAN) \
{\
	VE_PROFILE_FUNCTION(__VA_ARGS__); \
}


#else

#define VE_PROFILE_VULKAN_FUNCTION(...)

#endif // VE_PROFILE_VULKAN
