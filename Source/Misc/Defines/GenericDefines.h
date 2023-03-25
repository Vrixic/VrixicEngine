#pragma once
#include <string>
#include <memory>
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

typedef std::string VString;

template<typename T>
using TUniquePtr = std::unique_ptr<T>;

template<typename T, typename ... Args>
constexpr TUniquePtr<T> CreateUniquePointer(Args&& ... inArgs)
{
	return std::make_unique<T>(std::forward<Args>(inArgs)...);
}

#define BIT_SHIFT_LEFT(x) (1 << x)

#define VE_BIND_EVENT_FUNC(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

