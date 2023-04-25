/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Runtime/Memory/Core/MemoryManager.h>

/**
* Generic Iterator for a non-intrusive single linked list
*	Does not support operator-- (Going reverse on a list), can only traverse forward 
*/
template<typename ListType, typename ListNode, typename ListNodeType>
class VRIXIC_API TGenericNonIntrusiveSingleListIterator
{
public:
	TGenericNonIntrusiveSingleListIterator(ListType& inListType)
		: List(inListType)
	{
		CurrentNode = List.Head;
	}

public:
	/**
	* ----------------------------------------------------------------------------
	* ---------------------------Operator Overloading-----------------------------
	* ----------------------------------------------------------------------------
	*/

	/**
	* Moves iterator to the next node in the list
	* @returns TGenericNonIntrusiveSingleListIterator& - The next slot of the iterator that contains next node
	*/
	TGenericNonIntrusiveSingleListIterator& operator++()
	{
		// Check to see if we reached the end of the array, if so, break as they should not be allowed to go further! (Maybe should add a safe case)
		VE_ASSERT(CurrentNode != List.Tail || CurrentNode != nullptr);

		CurrentNode = (*CurrentNode)->GetNextNode();
		return *this;
	}

	/**
	* @returns ListNodeType& - a reference to the current node
	*/
	ListNodeType& operator*() const
	{
		return CurrentNode;
	}

	/**
	* @returns ListNodeType* - a pointer to the current node
	*/
	ListNodeType* operator->() const
	{
		return &CurrentNode;
	}

	/**
	* Overloaded == for iterator
	* @returns bool - true if this iterator's CurrentNode and list equals inRHS's, false otherwise
	*/
	inline bool operator==(const TGenericNonIntrusiveSingleListIterator& inRHS)
	{
		return &List == &inRHS.List && CurrentNode == inRHS.CurrentNode;
	}

	/**
	* Overloaded != for iterator
	* @returns bool - true if this iterator's CurrentNode and list are not equal inRHS's, false otherwise
	*/
	inline bool operator!=(const TGenericNonIntrusiveSingleListIterator& inRHS)
	{
		return &List != &inRHS.List || CurrentNode != inRHS.CurrentNode;
	}

public:
	/**
	* Resets the iterator to the beginning of the list
	*/
	void Reset()
	{
		CurrentNode = List.Head;
	}

	/**
	* Removes current node from the list
	* After removal, pointer is pointing to the next element in the list if one exists
	* If !IsFinished() then, it'll successfully remove an element, otherwise it will not
	*/
	void Remove()
	{
		// If the list is at the very end, cannot remove as index is beyond removal stage 
		if (IsFinished())
		{
			return;
		}

		ListNode** NodeToRemove = CurrentNode;
		CurrentNode = (*NodeToRemove)->GetNextNode();
		List.RemoveNode(NodeToRemove);
	}

	/**
	* Goes straight to tail, end of list
	*/
	void GoToTail()
	{
		CurrentNode = List.Tail;
	}

	/**
	* @returns ListNodeType& - the current element iterator is at
	*/
	ListNodeType& GetCurrentElement() const
	{
		return (*CurrentNode)->GetValue();
	}

	/**
	* @returns bool - if the iterator is at the end of the list
	*/
	bool IsFinished()
	{
		return CurrentNode == List.Tail;
	}

	/**
	* @returns ListNode - the current node the iterator is at
	*/
	ListNode GetCurrentNode() const
	{
		return CurrentNode;
	}

private:
	ListType& List;
	ListNode** CurrentNode;
};

/**
* Represents a Non-Intrusive (Storing only the element passed in) Linked List implementation
* Just like std::list (Link Lists), Collection not stored contigously
* Non-Intusive linked list implementation
*/
template<class NodeType>
class VRIXIC_API TLinkedList
{
public:
	TLinkedList()
		: Head(nullptr), Tail(nullptr), Size(0) { }

	virtual ~TLinkedList() 
	{ 
		TSingleListIterator Iterator = Begin();
		while (!Iterator.IsFinished())
		{
			Iterator.Remove();
		}
	}

public:
	/**
	* Addition/Insertion/Removal of Nodes 
	*/

	/**
	* Add an element to the beginning of the list
	* 
	* @param inNodeType - the element to add to the list
	* @returns bool - if the element was successfully added to the list or not 
	*/
	bool AddHead(const NodeType& inNodeType)
	{
		TLinkedListNode** Node = MemoryManager::Get().MallocAligned<TLinkedListNode>(sizeof(TLinkedListNode));
		(*Node)->Value = inNodeType;
		(*Node)->NextNode = nullptr;
		return AddHead(Node);
	}

	/**
	* Add a node to the beginning of the list
	*
	* @param inNewNode - the node to add to the list
	* @returns bool - if the node was successfully added to the list or not
	*/
	bool AddHead(TLinkedListNode** inNewNode)
	{
		// Cannot allow nullptr nodes to be added 
		if (inNewNode == nullptr)
		{
			return false;
		}

		// If we have an existing head, make the new node head
		if (Head != nullptr)
		{
			(*inNewNode)->NextNode = Head;
			Head = inNewNode;
			Tail = inNewNode;
		}
		else
		{
			Head = inNewNode;
			Tail = inNewNode;
		}

		Size++;
		return true;
	}

	/**
	* Add an element to the end of the list
	*
	* @param inNodeType - the element to add to the list
	* @returns bool - if the element was successfully added to the list or not
	*/
	bool AddTail(const NodeType& inNodeType)
	{
		TLinkedListNode** Node = MemoryManager::Get().MallocAligned<TLinkedListNode>(sizeof(TLinkedListNode));
		(*Node)->Value = inNodeType;
		(*Node)->NextNode = nullptr;
		return AddTail(Node);
	}

	/**
	* Add a node to the end of the list
	*
	* @param inNewNode - the node to add to the list
	* @returns bool - if the node was successfully added to the list or not
	*/
	bool AddTail(TLinkedListNode** inNewNode)
	{
		// Cannot allow nullptr nodes to be added 
		if (inNewNode == nullptr)
		{
			return false;
		}

		// If we have an existing tail, make the new node tail
		if (Tail != nullptr)
		{
			(*Tail)->NextNode = inNewNode;
			Tail = inNewNode;
		}
		else
		{
			Head = inNewNode;
			Tail = inNewNode;
		}

		Size++;
		return true;
	}

	/**
	* Insert a value after the specified node into the list
	* 
	* @param inNodeType - the value to insert
	* @param inNodeToInsertAfter - the node the new value will be inserted after 
	*/
	bool InsertAfter(const NodeType& inNodeType, TLinkedListNode** inNodeToInsertAfter = nullptr)
	{
		TLinkedListNode** Node = MemoryManager::Get().MallocAligned<TLinkedListNode>(sizeof(TLinkedListNode));
		(*Node)->Value = inNodeType;
		(*Node)->NextNode = nullptr;
		return InsertAfter(Node, inNodeToInsertAfter);
	}

	/**
	* Insert a node after the specified node into the list
	*
	* @param inNewNode - the node to insert
	* @param inNodeToInsertAfter - the node the new node will be inserted after
	*/
	bool InsertAfter(TLinkedListNode** inNewNode, TLinkedListNode** inNodeToInsertAfter = nullptr)
	{
		if (inNewNode == nullptr)
		{
			return false;
		}

		// Insert at tail 
		bool bInsertAtTail = inNodeToInsertAfter == Tail || inNodeToInsertAfter == nullptr;
		if (bInsertAtTail)
		{
			return AddTail(inNewNode);
		}

		// Insert node after the param
		(*inNewNode)->NextNode = (*inNodeToInsertAfter)->NextNode;
		(*inNodeToInsertAfter)->NextNode = inNewNode;

		Size++;
		return true;
	}

	/**
	* Finds a node in the list by the specified value
	* 
	* @param inElementToFind - the element to be found
	* @returns TLinkedListNode** - a pointer* to the node that contains the element specified, nullptr if it wasn't found 
	*/
	TLinkedListNode** FindNode(const NodeType& inElementToFind) const 
	{
		TLinkedListNode** FoundNode = nullptr;
		TLinkedListNode** CurrentNode = Head;

		while (CurrentNode != nullptr)
		{
			if ((*CurrentNode)->GetValue() == inElementToFind)
			{
				FoundNode = CurrentNode;
				break;
			}

			CurrentNode = (*CurrentNode)->NextNode;
		}

		return FoundNode;
	}

	/**
	* Removes a node in the list
	*
	* @param inNodeToRemove - the node to remove 
	* @returns bool - if node was successfuly removed
	*/
	bool RemoveNode(TLinkedListNode** inNodeToRemove)
	{
		if (inNodeToRemove == nullptr)
		{
			return false;
		}

		if (inNodeToRemove == Head)
		{
			Head = (*inNodeToRemove)->NextNode;
		}
		else 
		{
			TLinkedListNode** PrevNode = Head;
			TLinkedListNode** CurrNode = Head;

			while (CurrNode != nullptr)
			{
				PrevNode = CurrNode;
				CurrNode = (*CurrNode)->NextNode;

				if (CurrNode == inNodeToRemove)
				{
					if (CurrNode == Tail)
					{
						Tail = PrevNode;
					}

					(*PrevNode)->NextNode = (*CurrNode)->NextNode;
					break;
				}
			}
		}

		// Free node memory 
		MemoryManager::Get().Free((void**)inNodeToRemove);

		// Update node count 
		Size--;

		return true;
	}

	/**
	* Removes a node in the list by index, 0 is first node 
	*
	* @param inIndex - the index to remove
	* @returns bool - if node was successfuly removed 
	*/
	bool RemoveNodeAt(int32 inIndex)
	{
		if (inIndex < 0 || inIndex > (Size - 1))
		{
			return false;
		}

		TLinkedListNode** NodeToRemove = Head;

		if (inIndex == 0) // remove head
		{
			Head = (*NodeToRemove)->NextNode;
		}
		else // remove all other nodes 
		{
			NodeToRemove = (*Head)->NextNode;
			TLinkedListNode** NodeToRemovePrev = Head;

			for (int32 i = 1; i < inIndex; ++i)
			{
				if (i == (inIndex - 1))
				{
					NodeToRemovePrev = NodeToRemove;
				}

				NodeToRemove = (*NodeToRemove)->NextNode;
			}

			// Update links then remove node 
			(*NodeToRemovePrev)->NextNode = (*NodeToRemove)->NextNode;

			if (Tail == NodeToRemove)
			{
				Tail = NodeToRemovePrev;
			}
		}

		// Free node memory 
		MemoryManager::Get().Free((void**)NodeToRemove);

		// Update node count 
		Size--;

		return true;
	}

	/**
	* @returns TSingleListIterator - iterator starting at the beginning of the list
	*/
	TSingleListIterator Begin()
	{
		return CreateIterator();
	}

	/**
	* Creates a iterator for this TLinkedList
	* @returns TSingleListIterator - The iterator assocaited with this List
	*/
	TSingleListIterator CreateIterator()
	{
		return TSingleListIterator(*this);
	}

	/**
	* Creates a const iterator for this TLinkedList
	* @returns TSingleListIterator - The const iterator assocaited with this List
	*/
	TConstSingleListIterator CreateConstIterator() const
	{
		return TConstSingleListIterator(*this);
	}

	uint32 Count() const
	{
		return Size;
	}

public:
	template<typename ListType, typename ListNode, typename ListNodeType>
	friend class TGenericNonIntrusiveSingleListIterator;

	class TLinkedListNode
	{
	public:
		TLinkedListNode(const NodeType& inNodeType)
			: Value(inNodeType), NextNode(nullptr) { }

	public:
		/**
		* @returns TLinkedListNode* - the next node this node is pointing to
		*/
		const TLinkedListNode** GetNextNode() const
		{
			return NextNode;
		}

		/**
		* @returns TLinkedListNode* - the next node this node is pointing to
		*/
		TLinkedListNode** GetNextNode()
		{
			return NextNode;
		}

		/**
		* @returns NodeType& - value of the node
		*/
		const NodeType& GetValue() const
		{
			return Value;
		}

		/**
		* @returns NodeType& - value of the node
		*/
		NodeType& GetValue()
		{
			return Value;
		}

	public:
		template<class NodeType>
		friend class TLinkedList;

	protected:
		/** Current node value*/
		NodeType Value;

		/** A pointer pointing to the next node in the list */
		TLinkedListNode** NextNode;
	};

	typedef TGenericNonIntrusiveSingleListIterator<TLinkedList, TLinkedListNode, NodeType> TSingleListIterator;
	typedef TGenericNonIntrusiveSingleListIterator<const TLinkedList, const TLinkedListNode, const NodeType> TConstSingleListIterator;

private:
	/** The head node pointing to the first element in the list */
	TLinkedListNode** Head;

	/** The tail node pointing to the last element in the list */
	TLinkedListNode** Tail;

	/** Count of nodes in the list */
	uint32 Size;
};
