#include"BplusTree.h" 
#include <queue>

Keytype getMin(Node* root);

bool BplusTree::search(const Keytype target)
{	
	InternalNode* p = reinterpret_cast<InternalNode*>(_root);    
	while(!(p->isLeaf)) //当p一直为内部结点
    {                
        int r = p->key.size() - 1;
        while(r >= 0 && p->key[r] >= target)  r--; //在p指向的结点的键值中找到不大于target的最大key, r代表秩rank, 即位置
		if(r >= 0 && p->key[r] == target) //若target不会比最小的key还小(此时r=-1)且r处的值与target相等
			return true; //则找到并返回
		p = reinterpret_cast<InternalNode*>(p->child[r+1]);
    }				
	//下探到外部结点处了
    ExternalNode* q = reinterpret_cast<ExternalNode*>(p);
	//找到data中键值不大于target的元素的位置
    int r = q->data.size() - 1;
    while(r >= 0 && q->data[r].first >= target) r--;
	if(r >= 0 && q->data[r].first == target) 
		return true;
	else
		return false;
}

bool BplusTree::insert(const Datatype record)
{
//1、确认该元素不在B+树中
	if(search(record.first)) //如果找到了，说明不需要插入
		return false;
//2、找到该元素应该插入的位置并插入
	InternalNode* p = reinterpret_cast<InternalNode*>(_root);
	int k = record.first;
	while(!p->isLeaf)
    {        
        int r = p->key.size() - 1;        
        while(r >= 0 && p->key[r] >= k)  r--; //在p指向的结点的键值中找到不大于target的最大key, r代表秩rank, 即位置    
        p = reinterpret_cast<InternalNode*>(p->child[r+1]);
    }    
    ExternalNode* v = reinterpret_cast<ExternalNode*>(p);
   
    //如果叶结点中没有元素，则单独处理
    // {
    // cout << "target: " << record.first << endl;
    // cout << v->data.size() << endl;
    // }
    if(v->data.size() == 0)
    {
        v->data.push_back(record);        
        return true;
    }

    vector<Datatype>::iterator iter = v->data.begin();
    for (iter; iter != v->data.end(); iter++)
        if (iter->first > k) break;
    //while(iter != v->data.end() && iter->first <= k) iter++;
    //iter--;//得到的iter为所需要插入的位置的迭代器    
    v->data.insert(iter, record);
    
//3、检查是否上溢，若上溢进行第4步，否则返回
    if(v->data.size() <= L_order) //没有上溢
        return true;

//4、则开新的外部结点，将内容移过去        
    int s = L_order/2; //中间元素的位置

    //cout << "here1 " << s << endl;

    k = v->data[s].first;
    ExternalNode* u = new ExternalNode();
    iter  = v->data.begin();
    for(int i=0;i<s;i++)
       iter++;  //iter指向第s+1个元素
    for(int i=s;i<v->data.size();i++)
        u->data.push_back(v->data[i]);
    v->data.erase(iter, v->data.end());
    // cout << "v->data size:" << v->data.size() << endl;
    // cout << "u->data size:" << u->data.size() << endl;
    

    //cout << "here2" << endl;

//5、找到双亲结点在什么地方连到下面，并在那个地方插入新的索引和新的子树；若双亲结点不存在则新建一个
    InternalNode* q = reinterpret_cast<InternalNode*>(v->parent);
    if(!q) 
    {
        _root = q = new InternalNode();
        q->key.push_back(u->data[0].first);
        q->child.push_back(v);
        q->child.push_back(u);
        v->parent = q;
        u->parent = q;    
        return true;            
    } 
    u->parent = q;    
    vector<Keytype>::iterator iter2 = q->key.begin();
    vector<Node*>::iterator iter3 = q->child.begin();
    int r = q->key.size() - 1;
    while(r >= 0 && q->key[r] >= k) r--; //需要插入新键值的位置为r，需要插入新分支的位置为r+1
    r++;
    for(int i=0;i<r;i++)
        iter2++;
    for(int i=0;i<r+1;i++)
        iter3++;
    q->key.insert(iter2, k);
    q->child.insert(iter3, u);      

    //cout << "----------here3-------------" << endl;
          
    solveOverflow(q);
}

//解决内部结点上溢问题，使用递归
void BplusTree::solveOverflow(InternalNode* v)
{
//1、判断是否上溢，此处为递归基
    if(v->child.size() <= M_order) 
        return;
//2、创建新结点u，将中间元素之后的child和key移动过去
    InternalNode* u = new InternalNode();
    int s = (M_order +1)/ 2;
    //把v中相应元素移动过去
    for(int i=s;i<v->key.size();i++)
        u->key.push_back(v->key[i]);
    for(int i=s;i<v->child.size();i++)
        u->child.push_back(v->child[i]);
    
    //把v中相应元素删除
    vector<Keytype>::iterator iter1 = v->key.begin();
    vector<Node*>::iterator iter2 = v->child.begin();
    for(int i=0;i<s;i++)    iter1++, iter2++; //iter1和iter2指向第s+1个元素
    v->key.erase(--iter1, v->key.end());
    v->child.erase(iter2, v->child.end());    
//3、让u中的child指向u
    for(int i=0;i<u->child.size();i++)
        u->child[i]->parent = u;
    //cout << "|||-------------|||" << endl;
//4、找到父结点p，若父结点为空则创建新的父结点并赋给_root
    InternalNode* p = reinterpret_cast<InternalNode*>(v->parent);
    if(!p) {
        _root = p = new InternalNode();    
        p->child.push_back(v);
        p->child.push_back(u);
        v->parent = p;
        u->parent = p;
        p->key.push_back(getMin(u));
        return;
    }      
//5、在p中做相应插入
    int r = p->key.size()-1;
    for(;r >= 0 && p->key[r] > v->key[0];r--)
    ;
    r++;//插入到key的位置为r，插入到child的位置为r+1
    vector<Keytype>::iterator iter3 = p->key.begin();
    vector<Node*>::iterator iter4 = p->child.begin();
    for(int i=0;i<r;i++)
        iter3++, iter4++;
    iter4++;
    p->key.insert(iter3, getMin(u));
    p->child.insert(iter4, u);
//6、令u指向p
    u->parent = p;
//7、递归解决上溢
    solveOverflow(p);
}
Keytype renewNodeKey(Node* root)
{    
    if (root->isLeaf) return reinterpret_cast<ExternalNode*>(root)->data[0].first;
    InternalNode* temp = reinterpret_cast<InternalNode*>(root);
    for (int i = 0; i < temp->key.size(); i++) {
        temp->key[i] = renewNodeKey(reinterpret_cast<InternalNode*>(temp->child[i+1]));
    }
}
Keytype getMin(Node* root) {
    if (root->isLeaf) return reinterpret_cast<ExternalNode*>(root)->data[0].first;
    return getMin(reinterpret_cast<InternalNode*>(root)->child[0]);
}
void BplusTree::levelOrder() {
    queue<Node*> q;
    q.push(_root);
    int currentlineflag = 1;
    int nextlineflag = 0;
    int numbercount = 0;
    while(!q.empty()) {
        Node* temp = q.front();
        q.pop();
        if (!temp->isLeaf) {
            InternalNode* tempInternal = reinterpret_cast<InternalNode*>(temp); 
            cout << "| ";
            for (int i = 0; i < tempInternal->key.size(); i++)
                cout << tempInternal->key[i] << " ";
            cout << "| ";
            for (int i = 0; i < tempInternal->child.size(); i++)
                q.push(tempInternal->child[i]);
            numbercount++;
            nextlineflag += tempInternal->child.size();
            if (currentlineflag == numbercount) {
                cout << endl;
                currentlineflag = nextlineflag;
                nextlineflag = 0;
                numbercount = 0;
            }
        } else {
            ExternalNode* tempExternal = reinterpret_cast<ExternalNode*>(temp);
            cout << "| ";
            for (int i = 0; i < tempExternal->data.size(); i++)
                cout << tempExternal->data[i].first << " ";
            cout << " | ";
        }
    }
}

int main() {
    BplusTree BTree; 
    for (int i = 0; i < 21; i++)
        BTree.insert(make_pair(i, i*2));
    cout << "insert success!" << endl;
    BTree.levelOrder();
    cout << "\nsuccess!" << endl;

}