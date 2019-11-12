#ifndef BPLUSTREE_H
#define BPLUSTREE_H
#include"BplusNode.h" 
class BplusTree
{
private:    
    Node* _root;
    int size;    

public:
	BplusTree():size(0){_root = new ExternalNode();}
    bool search(const Keytype target); //查找元素
    bool insert(const Datatype record); //插入元素
    bool remove(const Keytype target); //删除元素	
    void levelOrder(); //层次遍历    
private:
    void solveOverflow(InternalNode* v); //解决内部结点上溢，用于插入函数
    void solveUnderflow(InternalNode *v);
    Keytype getMin(Node *root); //获得root孩子中最小的键值key    
};
#endif