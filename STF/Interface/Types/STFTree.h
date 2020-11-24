///
/// @brief 
///

#ifndef STFTREE_H
#define STFTREE_H

#include "STF/interface/Types/STFArray.h"

class STFTreeNode
   {
	private:
	   STFTreeNode *   parent;
		STFPointerArray children;

	public:
		STFTreeNode(void) : children(2) {parent = NULL; }
		virtual ~STFTreeNode();

		/// @brief returns whether this Node is a root
		bool IsRoot(void);

		/// @brief returns whether this Node is a leaf
		bool IsLeaf(void);

		/// @brief returns the Number of children
		/// @return the number of children
		uint32 GetChildCount(void);

		/// @brief returns the child at index index
		STFTreeNode * GetChild(uint32 index);

		/// @brief returns the Parent of this STFTreeNode
		STFTreeNode * GetParent(void);

		/// @brief adds a new child
		void AddChild(STFTreeNode * child);

		/// @brief sets the parent STFTreeNode.
		///
		/// this method does not delete the old parent!
		void SetParent(STFTreeNode * newparent);

		/// @brief sets a new child
		///
		/// this method does not delete the old child at this index!
		void SetChildAt(uint32 index, STFTreeNode * newchild);
		//void RemoveChild(STFTreeNode child);
		



   };

inline STFTreeNode::~STFTreeNode()
	{
	}

inline uint32 STFTreeNode::GetChildCount()
	{
	return children.Size();
	}

inline STFTreeNode * STFTreeNode::GetChild(uint32 index)
	{
	pointer p;
	STFTreeNode * node;
	children.ElementAt(index,p);
	node = (STFTreeNode*)p;
	return node;
	}

inline STFTreeNode * STFTreeNode::GetParent()
	{
	return parent;
	}

inline void STFTreeNode::AddChild(STFTreeNode * child)
	{
	children.Add(child);
	}

inline void STFTreeNode::SetParent(STFTreeNode * newparent)
	{
	parent = newparent;
	}

inline bool STFTreeNode::IsLeaf()
	{
	return 0 == children.Size();
	}

inline bool STFTreeNode::IsRoot()
	{
	return NULL == parent;
	}

#endif //STFTREE_H
