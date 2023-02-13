#pragma once
#include <Runtime/Memory/Core/MemoryManager.h>

/**
* @TODO: 
*		- Add Iterators for all type of Linked Lists
*		- TLinkedList: Add removal option
*		- TDoublyLinkedList: Implementation 
*/

/**
* Just like std::list (Link Lists), Collection not stored contigously
* Non-Intusive linked list implementation
*/

/**
* Represents a Non-Intrusive (Storing only the element passed in) Linked List implementation
*/
template<class NodeType>
class TLinkedList
{
	typedef TLinkedList<NodeType> ContainerType;
	
public:
	class TLinkedListNode
	{
	public:
		friend class TLinkedList;

	protected:
		/** Current node value*/
		NodeType Value;

		/** A pointer pointing to the next node in the list */
		TLinkedListNode** NextNode;

	public:
		TLinkedListNode(const NodeType& inNodeType)
			: Value(inNodeType), NextNode(nullptr) { }

	public:
		/**
		* @returns TLinkedListNode* - the next node this node is pointing to
		*/
		const TLinkedListNode* GetNextNode() const
		{
			return *NextNode;
		}

		/**
		* @returns TLinkedListNode* - the next node this node is pointing to
		*/
		TLinkedListNode* GetNextNode()
		{
			return *NextNode;
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
	}

private:
	/** The head node pointing to the first element in the list */
	TLinkedListNode** Head;

	/** The tail node pointing to the last element in the list */
	TLinkedListNode** Tail;

	/** Count of nodes in the list */
	int32 Size;

public:
	TLinkedList()
		: Head(nullptr), Tail(nullptr), Size(0) { }

	virtual ~TLinkedList() { }

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
		return Insert(Node, inNodeToInsertAfter);
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
		TLinkedListNode** CurrentNode = Head;
		while (CurrentNode != nullptr)
		{
			if ((*CurrentNode)->GetValue() == inElementToFind)
			{
				break;
			}

			CurrentNode = (*CurrentNode)->NextNode;
		}

		return CurrentNode;
	}
};

