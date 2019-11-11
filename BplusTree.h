#include"BplusNode.h" 
class BplusTree
{
private:    
    Node* _root;
    int size;    

public:
	BplusTree():size(0){_root = new ExternalNode();}
    bool search(const Keytype target);
    bool insert(const Datatype record);
    bool remove(const Keytype target);
	void solveOverflow(InternalNode* v);
    void levelOrder();
};