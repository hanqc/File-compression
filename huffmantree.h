#pragma once  

#include "heap.h"  
#include<assert.h>  


template<class T>
struct HuffmanTreeNode
{
	HuffmanTreeNode<T>* _left;
	HuffmanTreeNode<T>* _right;
	HuffmanTreeNode<T>* _parent;
	T _weight;

	HuffmanTreeNode(const T& x)
		:_weight(x)
		, _left(NULL)
		, _right(NULL)
		, _parent(NULL)
	{}
};

template<class T>
class HuffmanTree
{
	typedef HuffmanTreeNode<T> Node;

public:

	HuffmanTree()
		:_root(NULL)
	{}

	~HuffmanTree()
	{
		Destory(_root);
	}

	template <class T>
	struct NodeCompare
	{
		bool operator()(Node *l, Node *r)
		{
			return l->_weight < r->_weight;
		}
	};

public:
	void CreatTree(const T* a, size_t size, const T& invalid)
	{
		assert(a);
		Heap<Node*, NodeCompare<T>> minHeap;
		for (size_t i = 0; i < size; ++i)
		{
			if (a[i] != invalid)
			{
				Node* node = new Node(a[i]);
				minHeap.Push(node);
			}
		}

		while (minHeap.Size() > 1)
		{
			Node* left = minHeap.Top();
			minHeap.Pop();
			Node* right = minHeap.Top();
			minHeap.Pop();

			Node* parent = new Node(left->_weight + right->_weight);
			parent->_left = left;
			parent->_right = right;
			left->_parent = parent;
			right->_parent = parent;

			minHeap.Push(parent);
		}

		_root = minHeap.Top();
	}

	Node* GetRootNode()
	{
		return _root;
	}

	void Destory(Node* root)
	{
		if (root)
		{
			Destory(root->_left);
			Destory(root->_right);
			delete root;
			root = NULL;
		}
	}

private:
	HuffmanTreeNode<T>* _root;
};

void TestHuffmanTree()
{
	int a[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	HuffmanTree<int> hf;
	hf.CreatTree(a, 10, -1);
}