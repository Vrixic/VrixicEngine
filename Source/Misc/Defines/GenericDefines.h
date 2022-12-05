#pragma once
#include <string>

/* unsigned int 8-bit */
typedef unsigned char		uint8;

/* unsigned int 16-bit */
typedef unsigned short		uint16;

/* unsigned int 32-bit */
typedef unsigned int		uint32;

/* unsigned int 64-bit */
typedef unsigned long		uint64;

/* signed int 8-bit */
typedef signed char			int8;

/* signed int 16-bit */
typedef signed short		int16;

/* signed int 32-bit */
typedef signed int			int32;

/* signed int 64-bit */
typedef signed long			int64;
#pragma once

#define _DEBUG !NDEBUG

/* Assertion */
#if _DEBUG
#define ASSERTIONS_ENABLED 1
#endif

#if ASSERTIONS_ENABLED
// define some inline assembly that causes a break
// into the debugger -- this will be different on each
// target CPU
#define debugBreak() __debugbreak()
// check the expression and fail if it is false 
#define ASSERT(expr)							 \
  if(expr) { }									\
  else											\
 {												\
	debugBreak();								\
 }
#else
#define ASSERT(expr) // evaluates to nothing
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

typedef std::string VString;

