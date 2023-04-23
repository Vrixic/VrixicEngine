/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Misc/Defines/GenericDefines.h>
#include <Misc/Logging/Log.h>

/* Assertion */
#if _DEBUG
#define ASSERTIONS_ENABLED 1
#endif

#if ASSERTIONS_ENABLED

// define some inline assembly that causes a break
// into the debugger -- this will be different on each
// target CPU
#if defined(PLATFORM_WINDOWS)
#define DEBUG_BREAK() __debugbreak()
#elif defined(PLATFORM_MAC)
#define DEBUG_BREAK __builting_debugtrap();
//#elif defined(PLATFORM_LINUX)
#else
#define DEBUG_BREAK __builting_trap();
#endif

// check the expression and fail if it is false 
#define ASSERT(expr, ...)					        \
  if(expr) { }									    \
  else											    \
 {												    \
	VE_CORE_LOG_FATAL(__VA_ARGS__);	                \
    DEBUG_BREAK();                                  \
 }

#else
#define ASSERT(expr, message) // evaluates to nothing
#endif

/* Compile time assertion, checks if were using cpp11 if so use assert def*/
#ifdef __cplusplus
#if __cplusplus >= 201103L
#define STATIC_ASSERT(expr) \
   static_assert(expr, \
     "static assert failed:" \
     #expr)
#else
#define STATIC_ASSERT(expr)
#endif
#endif

// Engine defined asserts 
#if _DEBUG
#define VE_ASSERT(expr, ...) ASSERT(expr, __VA_ARGS__)
#define VE_STATIC_ASSERT(expr, message) STATIC_ASSERT(expr)
#else
#define VE_ASSERT(expr, message, ...) 
#define VE_STATIC_ASSERT(expr, message) 
#endif // _DEBUG

