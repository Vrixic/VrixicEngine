#pragma once
#include <Runtime/Memory/Core/MemoryManager.h>


/**
* A Generic iterator for all indexed based container types
*/
template<typename ContainerType, typename ContainerElementType>
class TGenericIndexedContainerIterator
{
private:
	ContainerType& Container;
	int32 Index;

public:
	TGenericIndexedContainerIterator(ContainerType& inContainer)
		: Container(inContainer), Index(0) { }

public:
	/**
	* ----------------------------------------------------------------------------
	* ---------------------------Operator Overloading-----------------------------
	* ----------------------------------------------------------------------------
	*/

	/** 
	* Moves iterator to the next element in the container
	* @returns TGenericIndexedContainerIterator& - The next slot of the iterator that contains next element
	*/
	TGenericIndexedContainerIterator& operator++()
	{
		++Index;
		return *this;
	}

	/**
	* Moves iterator to the previous element in the container
	* @returns TGenericIndexedContainerIterator& - The next slot of the iterator that contains next element
	*/
	TGenericIndexedContainerIterator& operator--()
	{
		--Index;
		return *this;
	}

	/**
	* @returns ContainerElementType& - a reference to the current element 
	*/
	ContainerElementType& operator* () const
	{
		return Container[Index];
	}

	/**
	* @returns ContainerElementType* - a pointer to the current element
	*/
	ContainerElementType* operator->() const
	{
		return &Container[Index];
	}

public:
	/**
	* Resets the iterator to the beggining of the container
	*/
	void Reset()
	{
		Index = 0;
	}

	/**
	* Moves the iterator to the end of the container
	*/
	void GoToEnd()
	{
		Index = Container.Count();
	}

	/**
	* @returns ContainerElementType& - the current element iterator is at 
	*/
	ContainerElementType& GetCurrentElement() const
	{
		return Container[Index];
	}

	/**
	* @returns bool - if the iterator is at the end of the container 
	*/
	bool IsFinished()
	{
		return Index == Container.Count();
	}

	/**
	* @returns int32 - the current index the iterator is at
	*/
	int32 GetCurrentIndex() const
	{
		return Index;
	}
};

/**
* Defines a Dynamic Array (std::vector<>)
* 
* Can only Grow | Cannot Shrink
* RAII 
*/
template <typename ElementType>
class TArray
{
public:
	typedef TGenericIndexedContainerIterator<TArray, ElementType> TArrayIterator;
	typedef TGenericIndexedContainerIterator<const TArray, const ElementType> TConstArrayIterator;

private:
	/** Amount of elements in the array currently */
	uint32 Size;

	/** Total amount of elements that array can fit */
	uint32 Capacity;

	/** Handle to the memory of the array */
	ElementType** MemoryHandle;
public:
	TArray()
		: Size(0), Capacity(0), MemoryHandle(nullptr) {  }

	TArray(uint32 inReserveAmount)
	{
		Size = 0;
		Capacity = inReserveAmount;

		uint32 SizeInBytes = sizeof(ElementType) * Capacity;
		MemoryHandle = MemoryManager::Get().MallocAligned<ElementType>(SizeInBytes);
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
	const ElementType& operator[](uint32 inIndex) const
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
	ElementType& operator[](uint32 inIndex) 
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
	void Add(const ElementType& inData)
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
	bool Find(const ElementType& inObjectToFind, int32& outIndex) const 
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
	bool FindReversed(const ElementType& inObjectToFind, int32& outIndex) const
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
		uint32 SizeInBytes = sizeof(ElementType) * Capacity;

		// allocate the memory 
		ElementType** NewMemoryHandle = MemoryManager::Get().MallocAligned<ElementType>(SizeInBytes);

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

	/**
	* Creates a iterator for this TArray
	* @returns TArrayIterator - The iterator assocaited with this array 
	*/
	TArrayIterator CreateIterator()
	{
		return TArrayIterator(*this);
	}

	/**
	* Creates a const iterator for this TArray
	* @returns TConstArrayIterator - The const iterator assocaited with this array
	*/
	TConstArrayIterator CreateConstIterator()
	{
		return TConstArrayIterator(*this);
	}

public:
	/**
	* @returns uint32 - count of elements in the array
	*/
	inline uint32 Count() const
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
	inline const ElementType* Data() const 
	{
		return (*MemoryHandle);
	}
};
