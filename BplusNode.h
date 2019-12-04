#ifndef BPLUSNODE_H
#define BPLUSNODE_H
#include<vector> 
#include<iostream>
#define _order 5 //B+树的阶数
#define _L (_order - 1) //结点关键码最大个数
#define _M _order //结点分支最大个数
using namespace std;
typedef int Keytype; //键值类型
typedef int Valuetype; //数值类型
typedef pair<Keytype, Valuetype> Datatype; //数据类型

//普通结点
struct Node
{
    Node* parent; //双亲结点
    bool isLeaf; //是否为叶结点，即是否为外部结点，通过该变量判断结点类型
    Node(bool flag):parent(NULL), isLeaf(flag){}       
};

//内部结点，继承普通结点 
struct InternalNode: public Node
{
    vector<Keytype> key; //存放关键码key
    vector<Node*> child; //存放孩子/分支 
    InternalNode(): Node(false){}  
};

//外部结点，继承普通结点
struct ExternalNode: public Node
{
    vector<Datatype> data; //存放数据
    ExternalNode(): Node(true){}
};
#endif