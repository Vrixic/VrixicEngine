#pragma once

#include <Core/Core.h>
#include <Misc/Defines/MemoryProfilerDefines.h>

/**
* @TODO: Offer heap alignment 
*/

/**
* Information on how the memory block is spliced
*/
struct VRIXIC_API MemoryPage
{
	/** The size of the memory, used as offset to the end of memory */
	ulong32 MemorySize;

	/** Amount of bytes form HeapStartPointer to MemoryStartPointer */
	ulong32 OffsetFromHeapStart;

	/** The pointer pointing to the start of the memory / pointing to data */
	uint8* Data;
};

/**
* A chunk of memory on the Heap (Pool of memory)
* Does not defragment by itself
*/
template<typename T>
class VRIXIC_API TMemoryHeap
{
	/** Pointer/Handle to the memory */
	uint8* MemoryHandle;

	/** Pointer to the end of Used memory */
	uint8* MemoryUsedPtr;

	/** The size of the allocated memory in bytes */
	uint64 HeapSize;

	/** Amount of memory in use in bytes */
	uint64 MemoryUsed;

	/** Amount of memory used on the Heap, from start to end (MemoryHandle to MemoryUsedPtr) */
	uint64 HeapUsed;

	/** Count of all allocations made to this heap */
	uint64 MemoryAllocationCount;

	/** Alignment added to the MemoryHandle Pointer */
	uint64 Alignment;

public:
	TMemoryHeap()
		: MemoryHandle(nullptr), MemoryUsedPtr(nullptr), MemoryUsed(0), MemoryAllocationCount(0),
		HeapUsed(0), Alignment(0), HeapSize(0)
	{

	}

	~TMemoryHeap()
	{
#if VE_PROFILE_MEMORY_HEAP
		VE_PROFILE_FUNCTION();
#endif // VE_PROFILE_MEMORY_HEAP


		Flush();
	}

public:
	/*
	* Allocates T count objects(since class is Template)
	*
	* @param inCount - count of T objects to allocate
	* @param inAlignment - the alignment of the heap (default = sizeof(T))
	*/
	void AllocateByCount(ulong32 inCount, uint64 inAlignment = sizeof(T))
	{
#if VE_PROFILE_MEMORY_HEAP
		VE_PROFILE_FUNCTION();
#endif // VE_PROFILE_MEMORY_HEAP

		uint64 SizeInBytes = sizeof(T) * inCount;
		AllocateByBytes(SizeInBytes);
	}

	/**
	* Allocates heap by bytes
	*
	* @param inSizeInBytes - amount of bytes to allocate
	* @param inAlignment - alignment of the heap
	* @warning calculate whole block size for T if (T != a byte)
	*/
	void AllocateByBytes(uint64 inSizeInBytes, uint64 inAlignment = sizeof(T))
	{
#if VE_PROFILE_MEMORY_HEAP
		VE_PROFILE_FUNCTION();
#endif // VE_PROFILE_MEMORY_HEAP

		HeapSize = inSizeInBytes;

		inSizeInBytes += inAlignment;
		uint8* RawMemoryPtr = new uint8[inSizeInBytes];
		uint8* AlignedPtr = AlignPointerAndShift(RawMemoryPtr, inAlignment);

		MemoryHandle = AlignedPtr;
		MemoryUsedPtr = AlignedPtr;
	}

	/**
	* Aligns pointer and stores the shift [-1] of the pointer
	*
	* @param inPtrToAlign - the pointer that will be aligned
	* @param inAlignment - n-byte alignment
	* @returns uint8* - aligned pointer
	*/
	uint8* AlignPointerAndShift(uint8* inPtrToAlign, uint64 inAlignment)
	{
#if VE_PROFILE_MEMORY_HEAP
		VE_PROFILE_FUNCTION();
#endif // VE_PROFILE_MEMORY_HEAP

		// Align the block, if their isn't alignment, shift it up the full 'align' bytes, so we always 
		// have room to store the shift 
		uint8* AlignedPtr = MemoryUtils::AlignPointer<uint8>(inPtrToAlign, inAlignment);
		if (AlignedPtr == inPtrToAlign)
		{
			AlignedPtr += inAlignment;
		}

		// Determine the shift, and store it for later when freeing
		// (This works for up to 256-byte alignment.)
		intptr Shift = AlignedPtr - inPtrToAlign;
#if _DEBUG || _DEBUG_EDITOR
		ASSERT(Shift > 0 && Shift <= 256, "[MemoryHeap]: Invalid shift amount for memory address alignment!");
#endif

		AlignedPtr[-1] = static_cast<uint8>(Shift & 0xff);
		Alignment = Shift;

		return AlignedPtr;
	}

public:

	/**
	* Allocates memory, Does not call Constructor, sizeof(T) * inCountToAllocate = BytesAllocated
	*
	* @param inCountToAllocate - count of how many T objects to allocate
	* @return T* - pointer pointing to the memory location
	*/
	T* Malloc(ulong32 inCountToAllocate)
	{
#if VE_PROFILE_MEMORY_HEAP
		VE_PROFILE_FUNCTION();
#endif // VE_PROFILE_MEMORY_HEAP

		uint64 SizeInBytes = sizeof(T) * inCountToAllocate;

		// Check if we can allocate enough memory
#if _DEBUG | _DEBUG_EDITOR
		ASSERT((HeapUsed + SizeInBytes) < HeapSize, "[MemoryHeap]: Out of memory; Memory OverFlow!");
#endif

		uint8* PointerToMemory = MemoryUsedPtr;

		MemoryUsedPtr = MemoryUsedPtr + SizeInBytes;
		MemoryUsed += SizeInBytes;
		HeapUsed += SizeInBytes;

		MemoryAllocationCount++;

		return (T*)PointerToMemory;
	}

	/**
	* Resize the pool, allocates more memory, do not scale down,
	* Frees last memory heap
	*
	* @return uint8* pointer pointing to the new memory location
	*/
	T* ResizeAndFlushByBytes(uint64 inSizeInBytes, uint64 inAlignment = sizeof(T))
	{
#if VE_PROFILE_MEMORY_HEAP
		VE_PROFILE_FUNCTION();
#endif // VE_PROFILE_MEMORY_HEAP

		uint64 LastHeapSize = HeapSize;
		HeapSize = inSizeInBytes;

		inSizeInBytes += inAlignment;

#if _DEBUG | _DEBUG_EDITOR
		ASSERT(HeapSize > LastHeapSize, "[MemoryHeap]: Cannot shrink a memory heap; Memory heaps can only grow!");
#endif // _DEBUG | _EDITOR

		// Shift alignment to get the start of heap 
		uint8* ActualMemoryHandle = MemoryHandle - Alignment;

		uint8* RawMemoryPtr = new uint8[inSizeInBytes];
		uint8* AlignedPtr = AlignPointerAndShift(RawMemoryPtr, inAlignment);

		memcpy(AlignedPtr, MemoryHandle, HeapUsed);

		delete[] ActualMemoryHandle;

		MemoryHandle = AlignedPtr;
		MemoryUsedPtr = AlignedPtr + HeapUsed;

		return (T*)MemoryHandle;
	}

	/**
	* Calculated total memory in used currently
	*/
	void Free(ulong32 inSize)
	{
#if VE_PROFILE_MEMORY_HEAP
		VE_PROFILE_FUNCTION();
#endif // VE_PROFILE_MEMORY_HEAP

		MemoryUsed -= inSize;
	}

	/**
	* Flushs the Heap, but doesn't delete memory
	*/
	void FlushNoDelete()
	{
#if VE_PROFILE_MEMORY_HEAP
		VE_PROFILE_FUNCTION();
#endif // VE_PROFILE_MEMORY_HEAP

		MemoryUsed = 0;
		HeapUsed = 0;

		MemoryUsedPtr = MemoryHandle;
	}

	/**
	* Frees/deletes all memory
	*/
	void Flush()
	{
#if VE_PROFILE_MEMORY_HEAP
		VE_PROFILE_FUNCTION();
#endif // VE_PROFILE_MEMORY_HEAP

		if (MemoryHandle != nullptr)
		{
			// Shift alignment to get the start of heap 
			MemoryHandle -= Alignment;

			delete[] MemoryHandle;
			MemoryHandle = nullptr;
		}
	}

public:
	inline uint64 GetHeapSize() const
	{
		return HeapSize;
	}

	inline uint8* GetMemoryHandle() const
	{
		return MemoryHandle;
	}

	/**
	* @returns uint64 - memory in use, in bytes
	*/
	inline uint64 GetMemoryUsed() const
	{
		return MemoryUsed;
	}

	/**
	* @returns uint64 - memory used from start to current heap pointer (CurrentHeapPointer - StartHeapPointer)
	*/
	inline uint64 GetHeapUsed() const
	{
		return HeapUsed;
	}

	inline uint64 GetMemoryAllocationCount() const
	{
		return MemoryAllocationCount;
	}
};
