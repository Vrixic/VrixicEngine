#pragma once
#include "MemoryAllocater.h"

/**
* Stack Memory Alocater
*	- Allocates in a stack fashion
*	- Only allocate in blocks of T
*	- Offers block size allocations, making it very efficient and fast
*/
template<typename T>
class StackBlockAllocater : public MemoryAllocater
{
private:
	ulong32 Top;

public:
	explicit StackBlockAllocater() : MemoryAllocater(), Top(0) { }

	virtual ~StackBlockAllocater() { }

public:

	/**
	* Allocates T objects for stack to use
	*
	* @param inCount - the count of objects to create
	* @param inAlignment - alignment of memory
	*/
	virtual void Init(ulong32 inCount, ulong32 inAlignment = sizeof(T))
	{
		ulong32 Size = inCount * sizeof(T);
		MemoryAllocater::Init(Size, inAlignment);
	}

	/**
	* Allocates a block
	*
	* @param inSizeInBytes - how many bytes to allocate	*
	* @return uint32 - index to the block
	*/
	ulong32 Alloc()
	{
#if _DEBUG | _DEBUG_EDITOR
		ASSERT(MemoryUsed + sizeof(T) < (MemorySize + 1));
#endif
		MemoryUsed += sizeof(T);
		return Top++;
	}

	/**
	* Allocates a block and calls constructor
	*
	* @param inSizeInBytes - how many bytes to allocate	*
	* @return uint32 - index to the block
	*/
	ulong32 AllocConstruct()
	{
#if _DEBUG | _DEBUG_EDITOR
		ASSERT(MemoryUsed + sizeof(T) < (MemorySize + 1));
#endif
		uint8* RawMemPtr = (*MemoryHandle + MemoryUsed);
		T* MemHandle = (new (RawMemPtr) T()); // placement-new

		MemoryUsed += sizeof(T);

		return Top++;
	}

	/**
	* Frees the last allocated memory at the top of the stack
	*/
	virtual void Free()
	{
#if _DEBUG | _DEBUG_EDITOR
		ASSERT(MemoryUsed != 0);
#endif
		MemoryUsed -= sizeof(T);
		Top--;
	}

	/**
	* Flushes memory, doesn't release it
	*/
	virtual void Flush() override
	{
		MemoryAllocater::Flush();
		Top = 0;
	}

public:
	inline T* Get(ulong32 inIndex)
	{
		return &(((T*)(Data()))[inIndex]);
	}
};

/**
* Stack Allocater
*	- Allocates in a stack fashion
*/
class StackAllocater : public MemoryAllocater
{
public:
	typedef ulong32 Marker;

	explicit StackAllocater() : MemoryAllocater() { }

	virtual ~StackAllocater() { }

public:

	/**
	* Allocates T objects for stack to use
	*
	* @param inCount - the count of objects to create
	* @param inAlignment - alignment of memory
	*/
	template<typename T>
	void Init(ulong32 inSizeInBytes, ulong32 inAlignment = sizeof(T))
	{
		MemoryAllocater::Init(inSizeInBytes, inAlignment);
	}

	/**
	* Allocates a block
	*
	* @param inSizeInBytesToAllocate - how many bytes to allocate
	* @return T* - pointer to the memory location
	*/
	template<typename T>
	T* Alloc(ulong32 inSizeInBytesToAllocate)
	{
#if _DEBUG | _DEBUG_EDITOR
		ASSERT(MemoryUsed + inSizeInBytesToAllocate < (MemorySize + 1));
#endif
		uint8* RawMemoryPtr = ((*MemoryHandle) + MemoryUsed);
		MemoryUsed += inSizeInBytesToAllocate;

		return (T*)RawMemoryPtr;
	}

	/**
	* Allocates a block and calls constructor
	*
	* @param inSizeInBytes - how many bytes to allocate	*
	* @return uint32 - index to the block
	*/
	template<class T>
	void* AllocConstruct(ulong32 inSizeInBytesToAllocate)
	{
#if _DEBUG | _DEBUG_EDITOR
		ASSERT(MemoryUsed + inSizeInBytesToAllocate < (MemorySize + 1));
#endif
		uint8* RawMemPtr = (*MemoryHandle + MemoryUsed);
		T* MemHandle = (new (RawMemPtr) T()); // placement-new

		MemoryUsed += inSizeInBytesToAllocate;

		return void*;
	}

	/**
	* Frees the last allocated memory at the top of the stack
	* 
	* @param 'inMarker' - how much to free from top of stack
	*/
	virtual void FreeToMarker(Marker inMarker)
	{
#if _DEBUG | _DEBUG_EDITOR
		ASSERT(MemoryUsed != 0 && inMarker < MemoryUsed);
#endif
		MemoryUsed = inMarker;
	}

	/**
	* Flushes memory, doesn't release it.
	* AKA. Clear
	*/
	virtual void Flush() override
	{
		MemoryAllocater::Flush();
	}

public:
	/** 
	* @returns 'Marker' - the current top stack position
	*/
	Marker GetMarker() const
	{
		return MemoryUsed;
	}
};
