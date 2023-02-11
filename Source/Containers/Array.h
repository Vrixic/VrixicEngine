#pragma once
#include <Runtime/Memory/Core/MemoryManager.h>

/**
* Defines a Dynamic Array (std::vector<>)
* 
* Can only Grow | Cannot Shrink
* RAII 
*/
template <typename T>
class TArray
{
private:
	/** Amount of elements in the array currently */
	uint32 Size;

	/** Total amount of elements that array can fit */
	uint32 Capacity;

	/** Handle to the memory of the array */
	T** MemoryHandle;
public:
	TArray()
		: Size(0), Capacity(0), MemoryHandle(nullptr) {  }

	TArray(uint32 inReserveAmount)
	{
		Size = 0;
		Capacity = inReserveAmount;

		uint32 SizeInBytes = sizeof(T) * Capacity;
		MemoryHandle = MemoryManager::Get().MallocAligned<T>(SizeInBytes);
	}

	TArray(const TArray& other) = delete;
	TArray operator=(const TArray& other) = delete;

	~TArray()
	{
		if (MemoryHandle != nullptr)
		{
			MemoryManager::Get().Free((void**)MemoryHandle);
		}
	}

public:
	/**
	* @param inIndex - the element index to retreive 
	* @returns const T& - reference to the objects retreived by the index passed in
	*/
	const T& operator[](uint32 inIndex) const
	{
#if _DEBUG || _DEBUG_EDITOR || _EDITOR
		ASSERT(inIndex < Capacity && Size > inIndex);
#endif

		return (*MemoryHandle) + inIndex;
	}

	/**
	* @param inIndex - the element index to retreive
	* @returns T& - reference to the objects retreived by the index passed in
	*/
	T& operator[](uint32 inIndex) 
	{
#if _DEBUG || _DEBUG_EDITOR || _EDITOR
		ASSERT(inIndex < Capacity && Size > inIndex);
#endif

		return (*MemoryHandle)[inIndex];
	}

	/**
	* Adds an object T to the array 
	* @param inData - the data to add to the back of the array 
	*/
	void Add(const T& inData)
	{
		if (Size == Capacity)
		{
			Resize();
		}

		(*MemoryHandle)[Size] = inData;
		Size++;
	}

	/**
	* Finds the element passed in using Linear Search 
	* @param inObjectToFind - the object to find in the array 
	* @param outIndex - the index of the found element 
	* @returns bool - true if object was found in array, false otherwise
	* @note: 'outIndex' will only be valid if the return value is true
	*/
	bool Find(const T& inObjectToFind, int32& outIndex) const 
	{
		outIndex = -1;

		for (int32 i = 0; i < static_cast<int32>(Size); ++i)
		{
			if ((*MemoryHandle)[i] == inObjectToFind)
			{
				outIndex = i;
				return true;
			}
		}

		return false;
	}

	/**
	* Finds the element passed in using Linear Search, but begins from the end of the array 
	* @param inObjectToFind - the object to find in the array
	* @param outIndex - the index of the found element
	* @returns bool - true if object was found in array, false otherwise
	* @note: 'outIndex' will only be valid if the return value is true
	*/
	bool FindReversed(const T& inObjectToFind, int32& outIndex) const
	{
		outIndex = -1;

		for (int32 i = static_cast<int32>(Size) - 1; i >= 0; --i)
		{
			if ((*MemoryHandle)[i] == inObjectToFind)
			{
				outIndex = i;
				return true;
			}
		}

		return false;
	}

	/**
	* Resizes the array (can only grow)
	* @param inNewCapacity - the new capacity of the array
	* @note If 0 is passed, the capacity doubles itself
	*/
	void Resize(uint32 inNewCapacity = 0)
	{
		if (Capacity == 0)
		{
			Capacity = 1;
		}
		else if (inNewCapacity == 0)
		{
			Capacity *= 2;
		}

#if _DEBUG || _DEBUG_EDITOR || _EDITOR
		ASSERT(inNewCapacity < Capacity);
#endif
		// Calculate new size in bytes for the array 
		uint32 SizeInBytes = sizeof(T) * Capacity;

		// allocate the memory 
		T** NewMemoryHandle = MemoryManager::Get().MallocAligned<T>(SizeInBytes);

		// Check to see if we had already allocated memory before
		// if so, copy all of the elements from the old array to new array
		if (Size > 0)
		{
			for (uint32 i = 0; i < Size; ++i)
			{
				(*NewMemoryHandle)[i] = (*MemoryHandle)[i];
			}

			// Free old memory 
			MemoryManager::Get().Free((void**)MemoryHandle);
		}

		// Set memory handle to new one 
		MemoryHandle = NewMemoryHandle;
	}

	/**
	* Clears the array, does not deallocate the memory
	*/
	void Clear()
	{
		Size = 0;
	}

	/**
	* Flushes the array deallocating all memory used by it
	*/
	void Flush()
	{
#if _DEBUG || _DEBUG_EDITOR || _EDITOR
		ASSERT(MemoryHandle != nullptr);
#endif 

		Size = 0;
		Capacity = 0;
		MemoryManager::Get().Free((void**)MemoryHandle);
	}
public:
	/**
	* @returns uint32 - number of elements in the array
	*/
	inline uint32 Num() const
	{
		return Size;
	}

	/**
	* @returns uint32 - the total amount of elements the array can fit
	*/
	inline uint32 Max() const
	{
		return Capacity;
	}

	/**
	* @returns T* - pointer to the first element in the array 
	*/
	inline const T* Data() const 
	{
		return (*MemoryHandle);
	}
};
