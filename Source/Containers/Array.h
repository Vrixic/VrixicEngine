/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include "IteratorGenerics.h"
#include <Runtime/Memory/Core/MemoryManager.h>

/**
* A Generic iterator for all indexed based container types
*/
template<typename ContainerType, typename ContainerElementType>
class VRIXIC_API TGenericIndexedContainerIterator
{
public:
	TGenericIndexedContainerIterator(ContainerType& inContainer, EIteratorPointer inIteratorPointer = EIteratorPointer::Begin)
		: Container(inContainer)
	{
		switch (inIteratorPointer)
		{
		case EIteratorPointer::Begin:
			Index = 0;
			break;
		case EIteratorPointer::End:
			Index = Container.Count();
			break;
		default:
			Index = 0;
		}
	}

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
		// Check to see if we reached the end of the array, if so, break as they should not be allowed to go further! (Maybe should add a safe case)
		VE_ASSERT(Index != Container.Count());

		++Index;
		return *this;
	}

	/**
	* Moves iterator to the previous element in the container
	* @returns TGenericIndexedContainerIterator& - The next slot of the iterator that contains next element
	*/
	TGenericIndexedContainerIterator& operator--()
	{
		// Check to see if the start is reached, if it is, why go further? (Maybe should add a safe case)
		VE_ASSERT(Index != 0);

		--Index;
		return *this;
	}

	/**
	* @returns ContainerElementType& - a reference to the current element
	*/
	ContainerElementType& operator*() const
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

	/**
	* Overloaded == for iterator 
	* @returns bool - true if this iterator's index and container equals inRHS's, false otherwise 
	*/
	inline bool operator==(const TGenericIndexedContainerIterator& inRHS)
	{ 
		return &Container == &inRHS.Container && Index == inRHS.Index;
	}

	/**
	* Overloaded != for iterator
	* @returns bool - true if this iterator's index and container are not equal inRHS's, false otherwise
	*/
	inline bool operator!=(const TGenericIndexedContainerIterator& inRHS) 
	{ 
		return &Container != &inRHS.Container || Index != inRHS.Index; 
	}

public:
	/**
	* Resets the iterator to the beginning of the container
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
	* Removes current element from the array
	* After removal, pointer is pointing to the next element in the array if one exists 
	* If !IsFinished() then, it'll successfully remove an element, otherwise it will not
	*/
	void Remove()
	{
		// If the array is at the very end, cannot remove as index is beyond removal stage 
		if (IsFinished())
		{
			return;
		}

		Container.RemoveAt(Index);
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

private:
	ContainerType& Container;
	uint32 Index;
};

/**
* Defines a Dynamic Array (std::vector<>)
*
* RAII implementation (For memory)
* Array cannot shrink down (Not even when elements are removed from array)
* Instead of usual 'T' for templates, now making better template names such as ElementType (More descriptive)
*/
template <typename ElementType>
class VRIXIC_API TArray
{
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
		// Check to see if the index passed in is < our max elements we can have and its lower than the amount of 
		// elements currently stored
		VE_ASSERT(inIndex < Capacity && Size > inIndex);

		return (*MemoryHandle) + inIndex;
	}

	/**
	* @param inIndex - the element index to retreive
	* @returns T& - reference to the objects retreived by the index passed in
	*/
	ElementType& operator[](uint32 inIndex)
	{
		// Check to see if the index passed in is < our max elements we can have and its lower than the amount of 
		// elements currently stored
		VE_ASSERT(inIndex < Capacity && Size > inIndex); 

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
	* Removes the element at the specified index | doesn't resize, nor deallocate memory 
	* @param inIndexToRemove - the index of teh element to be removed 
	*/
	void RemoveAt(int32 inIndexToRemove)
	{
		// Check to see if the index passed in is < our max elements we can have and its lower than the amount of 
		// elements currently stored,also not a negative index 
		VE_ASSERT(inIndexToRemove < Capacity && Size > inIndexToRemove && inIndexToRemove > 0);

		// Instead of deallocating the memory, we can just lower the size and shift all of the element from right to left 
		for (int32 i = inIndexToRemove; i < Size - 1; ++i)
		{
			(*MemoryHandle)[i] = (*MemoryHandle)[i + 1];
		}

		// Decrement
		Size--;
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

		// Check to see if the resize if growing the array not shrinking, as it is not allowed 
		VE_ASSERT(inNewCapacity < Capacity);

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
		// If it memory is already freed, why free again?
		VE_ASSERT(MemoryHandle != nullptr);

		Size = 0;
		Capacity = 0;
		MemoryManager::Get().Free((void**)MemoryHandle);
	}

	/**
	* @returns TArrayIterator - iterator starting at the beginning of the array 
	*/
	TArrayIterator Begin()
	{
		return CreateIterator();
	}

	/**
	* @returns TArrayIterator - iterator starting at the end of the array
	*/
	TArrayIterator End()
	{
		return CreateIterator(EIteratorPointer::End);
	}

	/**
	* Creates a iterator for this TArray
	* @returns TArrayIterator - The iterator assocaited with this array
	*/
	TArrayIterator CreateIterator(EIteratorPointer inIteratorPointer = EIteratorPointer::Begin)
	{
		return TArrayIterator(*this, inIteratorPointer);
	}

	/**
	* Creates a const iterator for this TArray
	* @returns TConstArrayIterator - The const iterator assocaited with this array
	*/
	TConstArrayIterator CreateConstIterator(EIteratorPointer inIteratorPointer = EIteratorPointer::Begin) const
	{
		return TConstArrayIterator(*this, inIteratorPointer);
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
};
