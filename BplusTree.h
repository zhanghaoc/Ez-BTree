#ifndef BPLUSTREE_H
#define BPLUSTREE_H
#define DELAY 20 //延迟时间为20ms
#include"BplusNode.h" 
class BplusTree
{
private:    
    Node* _root; //根结点

public:
	BplusTree(){_root = new ExternalNode();}
    //下面的查找，插入，删除都有两个重载版本，一个是有时间，一个是没有时间的
    bool search(const Keytype target);
    bool search(const Keytype target, int& time); //查找元素

    bool insert(const Datatype record);
    bool insert(const Datatype record, int& time); //插入元素

    bool remove(const Keytype target);
    bool remove(const Keytype target, int& time); //删除元素	

    void levelOrder(); //层次遍历    
private:
    void solveOverflow(InternalNode* v); //解决内部结点上溢，用于插入函数
    void solveUnderflow(InternalNode *v); //解决内部结点下溢，用于删除函数
    Keytype getMin(Node *root); //获得root孩子中最小的键值key    
};
#endif