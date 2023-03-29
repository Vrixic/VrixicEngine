/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once

#include <Misc/Defines/GenericDefines.h>

#include <Optick/optick.h>

#define VE_PROFILE 1 && (_EDITOR || _DEBUG_EDITOR || _DEBUG)
#if VE_PROFILE

#define VE_PROFILE_BEGIN_SESSION(name)			OPTICK_FRAME(name)
#define VE_PROFILE_END_SESSION()			OPTICK_SHUTDOWN()

#define VE_PROFILE_FUNCTION(...)				OPTICK_EVENT(__VA_ARGS__)
#define VE_PROFILE_FRAME(...)					VE_PROFILE_BEGIN_SESSION(__VA_ARGS__)

#else

#define VE_PROFILE_BEGIN_SESSION(name) 
#define VE_PROFILE_END_SESSION(name) 
#define VE_PROFILE_FUNCTION(...) 

#endif