/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Core/Core.h>
#include <Misc/Defines/GenericDefines.h>

#include <unordered_map>

/**
* A map that holds Key-Value pairs with O(1) addition, removal, and search times, using hashing.
*
* | -- As of now creating a custom hash table would take time, so instead uses STL, but created a wrap around it, so
*			that later I can come I can change it to a custom hash table
*
* | -- By default the map does not preallocate memory, only when an element is added it will allocate more memory
* | -- Good usages of the map: pre allocating memory before using it, is always the better option
*/
template<typename KeyType, typename ValueType>
class VRIXIC_API TMap
{
private:
	std::unordered_map<KeyType, ValueType> Map;

	// Count of pair values stored in the map 
	int32 Size;

public:
	TMap() = default;
	TMap(TMap&&) = default;
	TMap(const TMap&) = default;
	TMap& operator=(TMap&&) = delete;
	TMap& operator=(const TMap&) = delete;

public:
	/**
	* Adds a value associated with a key
	*
	* @param inKey - the key associated with the value
	* @param inValue - the value associated with the key
	*/
	void Add(const KeyType& inKey, const ValueType& inValue)
	{
		Map.insert(std::make_pair(inKey, inValue));
		Size++;
	}

	/**
	* Adds a value associated with a key
	*
	* @param inKey - the key associated with the value
	* @param inValue - the value associated with the key
	*/
	void Add(const KeyType& inKey, ValueType& inValue)
	{
		Map.insert(std::make_pair(inKey, inValue));
		Size++;
	}

	/**
	* Removes all values by key, (Key will also get removed)
	*
	* @param inKey - the key to remove 
	* @return int32 - the number of values/elements removed that were associated with the key that was removed 
	*/
	int32 Remove(const KeyType& inKey)
	{
		int32 RemoveCount = Map.erase();
		if (RemoveCount != 0)
		{
			Size--;
		}

		return RemoveCount;
	}

	/**
	* Finds a value associated with a key and returns an iterator pointer to it
	* 
	* @param inKey - the key to find
	* @returns std::unordered_map<KeyType, ValueType>::iterator - the iterator to the value, if == to end it is null
	*/
	auto Find(KeyType& inKey)
	{
		return Map.find(inKey);
	}

	/**
	* Reserves an amount of values to be stored
	*
	* @param inAmountToReserve - the amount to reserve
	*/
	void Reserve(int32 inAmountToReserve)
	{
		Map.reserve(inAmountToReserve);
	}

public:
	inline int32 Count() const 
	{
		return Size;
	}

	inline auto End() const
	{
		return Map.end();
	}
};
