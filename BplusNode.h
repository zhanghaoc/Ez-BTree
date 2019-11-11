#include<vector> 
#include<iostream>
#define L_order 4
#define M_order 5
using namespace std;
typedef int Keytype;
typedef int Valuetype;
typedef pair<Keytype, Valuetype> Datatype;

struct Node
{
    Node* parent;     
    bool isLeaf;
    Node(bool flag):parent(NULL), isLeaf(flag){}       
};
struct InternalNode: public Node
{
    vector<Keytype> key;
    vector<Node*> child;    
    InternalNode(): Node(false){}  
};
struct ExternalNode: public Node
{
    vector<Datatype> data;
    ExternalNode(): Node(true){}
};