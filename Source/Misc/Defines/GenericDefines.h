#pragma once
#include <string>

/*
* unsigned int 8-bits, byte size: 1
*/
typedef unsigned char		uint8;

/*
* unsigned int 16-bits, byte size: 2
*/
typedef unsigned short		uint16;

/*
* unsigned int 32-bits, byte size: 4
*/
typedef unsigned int		uint32;

/**
* unsigned long 32-bits, byte size: 4
*/
typedef unsigned long		ulong32;

/**
* unsigned long long 64-bits, byte size: 8
*/
typedef unsigned long long  uint64;

/**
* signed int 8-bits, byte size: 1
*/
typedef signed char			int8;

/**
* signed int 16-bits, byte size: 2
*/
typedef signed short		int16;

/**
* signed int 32-bits, byte size: 4
*/
typedef signed int			int32;

/**
* signed int 32-bits, byte size: 4
*/
typedef signed long			long32;

/**
* signed int 64-bits, byte size: 8
*/
typedef signed long	long	int64;

/**
* unsigned int pointer 64-bits, byte size: 8
*/
typedef uint64             uintptr;

/**
* signed int pointer 64-bits, byte size: 8
*/
typedef int64               intptr;

#pragma once

// In debug mode 
#define _DEBUG !NDEBUG

// In editor in debug mode 
#define _DEBUG_EDITOR !NDEBUG

// In editor 
#define _EDITOR !NDEBUG

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

