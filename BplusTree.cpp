#include "BplusTree.h"
#include <queue>

Keytype getMin(Node *root);

bool BplusTree::search(const Keytype target)
{
    InternalNode *p = reinterpret_cast<InternalNode *>(_root);
    while (!(p->isLeaf)) //当p一直为内部结点
    {
        int r = p->key.size() - 1;
        while (r >= 0 && p->key[r] > target)
            r--;                           //在p指向的结点的键值中找到不大于target的最大key, r代表秩rank, 即位置
        //若target不会比最小的key还小(此时r=-1)且r处的值与target相等
        
        p = reinterpret_cast<InternalNode *>(p->child[r + 1]);
    }
    //下探到外部结点处了
    ExternalNode *q = reinterpret_cast<ExternalNode *>(p);
    //找到data中键值不大于target的元素的位置
    for (int i = 0; i < q->data.size(); i++)
        if (target == q->data[i].first)
            return true;
    return false;
}

bool BplusTree::insert(const Datatype record)
{
    //1、确认该元素不在B+树中
    if (search(record.first)) //如果找到了，说明不需要插入
        return false;
    //2、找到该元素应该插入的位置并插入
    InternalNode *p = reinterpret_cast<InternalNode *>(_root);
    int k = record.first;
    while (!p->isLeaf)
    {
        int r = p->key.size() - 1;
        while (r >= 0 && p->key[r] > k)
            r--; //在p指向的结点的键值中找到不大于target的最大key, r代表秩rank, 即位置
        p = reinterpret_cast<InternalNode *>(p->child[r + 1]);
    }
    ExternalNode *v = reinterpret_cast<ExternalNode *>(p);

    //如果叶结点中没有元素，则单独处理
    if (v->data.size() == 0)
    {
        v->data.push_back(record);
        return true;
    }

    vector<Datatype>::iterator iter = v->data.begin();
    for (iter; iter != v->data.end(); iter++)
        if (iter->first > k)
            break;
    //while(iter != v->data.end() && iter->first <= k) iter++;
    //iter--;//得到的iter为所需要插入的位置的迭代器
    v->data.insert(iter, record);

    //3、检查是否上溢，若上溢进行第4步，否则返回
    if (v->data.size() <= L_order) //没有上溢
        return true;

    //4、则开新的外部结点，将内容移过去
    int s = L_order / 2; //中间元素的位置

    k = v->data[s].first;
    ExternalNode *u = new ExternalNode();
    iter = v->data.begin();
    for (int i = 0; i < s; i++)
        iter++; //iter指向第s+1个元素
    for (int i = s; i < v->data.size(); i++)
        u->data.push_back(v->data[i]);
    v->data.erase(iter, v->data.end());


    //5、找到双亲结点在什么地方连到下面，并在那个地方插入新的索引和新的子树；若双亲结点不存在则新建一个
    InternalNode *q = reinterpret_cast<InternalNode *>(v->parent);
    if (!q)
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
    vector<Node *>::iterator iter3 = q->child.begin();
    int r = q->key.size() - 1;
    while (r >= 0 && q->key[r] > k)
        r--; //需要插入新键值的位置为r，需要插入新分支的位置为r+1
    r++;
    for (int i = 0; i < r; i++)
        iter2++;
    for (int i = 0; i < r + 1; i++)
        iter3++;
    q->key.insert(iter2, k);
    q->child.insert(iter3, u);

    solveOverflow(q);
    return true;
}

//解决内部结点上溢问题，使用递归
void BplusTree::solveOverflow(InternalNode *v)
{
    //1、判断是否上溢，此处为递归基
    if (v->child.size() <= M_order)
        return;
    //2、创建新结点u，将中间元素之后的child和key移动过去
    InternalNode *u = new InternalNode();
    int s = (M_order + 1) / 2;
    //把v中相应元素移动过去
    for (int i = s; i < v->key.size(); i++)
        u->key.push_back(v->key[i]);
    for (int i = s; i < v->child.size(); i++)
        u->child.push_back(v->child[i]);

    //把v中相应元素删除
    vector<Keytype>::iterator iter1 = v->key.begin();
    vector<Node *>::iterator iter2 = v->child.begin();
    for (int i = 0; i < s; i++)
        iter1++, iter2++; //iter1和iter2指向第s+1个元素
    v->key.erase(--iter1, v->key.end());
    v->child.erase(iter2, v->child.end());
    //3、让u中的child指向u
    for (int i = 0; i < u->child.size(); i++)
        u->child[i]->parent = u;

    //4、找到父结点p，若父结点为空则创建新的父结点并赋给_root
    InternalNode *p = reinterpret_cast<InternalNode *>(v->parent);
    if (!p)
    {
        _root = p = new InternalNode();
        p->child.push_back(v);
        p->child.push_back(u);
        v->parent = p;
        u->parent = p;
        p->key.push_back(getMin(u));
        return;
    }
    //5、在p中做相应插入
    int r = p->key.size() - 1;
    for (; r >= 0 && p->key[r] > v->key[0]; r--)
        ;
    r++; //插入到key的位置为r，插入到child的位置为r+1
    vector<Keytype>::iterator iter3 = p->key.begin();
    vector<Node *>::iterator iter4 = p->child.begin();
    for (int i = 0; i < r; i++)
        iter3++, iter4++;
    iter4++;
    p->key.insert(iter3, getMin(u));
    p->child.insert(iter4, u);
    //6、令u指向p
    u->parent = p;
    //7、递归解决上溢
    solveOverflow(p);
}
bool BplusTree::remove(const Keytype record)
{
    //1、确认该元素是否在B+树中
    if (!search(record)) //如果没找到，说明不需要删除
        return false;
    
    //2、找到该元素所在的位置并删除
    InternalNode *p = reinterpret_cast<InternalNode *>(_root);
    InternalNode *t = reinterpret_cast<InternalNode *>(_root);
    int r = p->key.size() - 1;
    while (!p->isLeaf)
    {
        t = p;
        r = p->key.size() - 1;
        while (r >= 0 && p->key[r] > record)
            r--; //在p指向的结点的键值中找到不大于target的最大key, r代表秩rank, 即位置
        r++;
        p = reinterpret_cast<InternalNode *>(p->child[r]);
    }
    ExternalNode *v = reinterpret_cast<ExternalNode *>(p);
    
    vector<Datatype>::iterator iter = v->data.begin();
    for (iter; iter != v->data.end(); iter++)
        if (iter->first == record)
            break;
    v->data.erase(iter);
    t->key[r-1] = v->data[0].first;

    //3.判断叶节点元素是否过少，若过少，进行第4步
    if (v->data.size() >= (L_order + 1) / 2)
        return true; //4->2, 5->3;

    //4.判断是否能从左右拿到元素。若无法拿到，进行第5步
    //r是数组下标 
    ExternalNode *tempExternalNode = NULL;
    if (r != 0) {
        tempExternalNode = reinterpret_cast<ExternalNode *>(t->child[r - 1]);
        if (tempExternalNode->data.size() > (L_order + 1) / 2) {
            v->data.insert(v->data.begin(), tempExternalNode->data[tempExternalNode->data.size()-1]);
            tempExternalNode->data.pop_back();
            t->key[r-1] = v->data[0].first;
            return true;
        }
    } 
    if (r != t->child.size() - 1) {
        tempExternalNode = reinterpret_cast<ExternalNode *>(t->child[r + 1]);
        if (tempExternalNode->data.size() > (L_order + 1) / 2) {
            v->data.insert(v->data.end(), tempExternalNode->data[tempExternalNode->data.size()-1]);
            tempExternalNode->data.erase(tempExternalNode->data.begin());
            return true;
        }
    }

    //5.合并相邻两个结点，并删除相关结点
    vector<Keytype>::iterator iter1 = t->key.begin();
    vector<Node *>::iterator iter2 = t->child.begin();
    for (int i = 0; i < r; i++)
        iter1++, iter2++;
    if (r != 0) {
        tempExternalNode = reinterpret_cast<ExternalNode *>(t->child[r - 1]);
        for (int i = 0; i < v->data.size(); i++)
            tempExternalNode->data.insert(tempExternalNode->data.end(), v->data[i]);
        iter1--;
        t->child.erase(iter2);
        t->key.erase(iter1);    
    } else if (r != t->child.size() - 1) {
        tempExternalNode = reinterpret_cast<ExternalNode *>(t->child[r + 1]);
        for (int i = v->data.size()-1; i >= 0; i--)
            tempExternalNode->data.insert(tempExternalNode->data.begin(), v->data[i]);
        t->key[r] = tempExternalNode->data[0].first;
        t->child.erase(iter2);
        t->key.erase(iter1);
    }
    solveDownflow(reinterpret_cast<InternalNode *>(p->parent));
    return true;
}
void BplusTree::solveDownflow(InternalNode *v)
{
    //1.检查是否结点内元素个数不够，此处为递归基
    //5-3,6-3
    if (v->child.size() >= (M_order + 1) / 2)
        return;
    
    //2.先查看是否能从相邻结点拿一个元素
    InternalNode *tempExternalNode = NULL;
    InternalNode* root = reinterpret_cast<InternalNode*>(v->parent);
    //v是_root的时候，检测child是否只剩一个元素，
    //若是，删去原来的root，用其child替代，这是树唯一减少高度的方式
    if (!root) {
        if (v->key.size() == 0) {
            _root = v->child[0];
            delete v;
        }
        return;
    }
    int r = 0;
    for (r = 0; r < root->child.size(); r++)
        if (root->child[r] == v) break;
    //先看左边是否有元素，同时判断个数是否符合要求，若符合，拿一个过来
    if (r != 0) {
        tempExternalNode = reinterpret_cast<InternalNode*>(root->child[r-1]);
        if (tempExternalNode->child.size() > (M_order+1)/2) {
            v->key.insert(v->key.begin(), getMin(v->child[0]));
            v->child.insert(v->child.begin(), tempExternalNode->child.back());
            tempExternalNode->key.pop_back();
            tempExternalNode->child.pop_back();
            return;
        }
    }
    //再看右边是否有元素，同时判断个数是否符合要求，若符合，拿一个过来
    if (r != root->child.size()-1) {
        tempExternalNode = reinterpret_cast<InternalNode*>(root->child[r+1]);
        if (tempExternalNode->child.size() > (M_order+1)/2) {
            v->child.insert(v->child.end(), tempExternalNode->child[0]);
            tempExternalNode->key.erase(tempExternalNode->key.begin());
            tempExternalNode->child.erase(tempExternalNode->child.begin());
            v->key.insert(v->key.end(), getMin(v->child.back()));
            return;
        }
    }
    //3.此时相邻node元素个数均不符合要求，将其中一个相邻node合并
    //同时删去相关的key和child
    vector<Keytype>::iterator iter1 = root->key.begin();
    vector<Node *>::iterator iter2 = root->child.begin();
    for (int i = 0; i < r; i++)
        iter1++, iter2++;
    if (r != 0) {
        tempExternalNode = reinterpret_cast<InternalNode *>(root->child[r - 1]);
        for (int i = 0; i < v->child.size(); i++) {
            tempExternalNode->child.insert(tempExternalNode->child.end(), v->child[i]);
            tempExternalNode->key.insert(tempExternalNode->key.end(), getMin(tempExternalNode->child.back()));
        }
        iter1--;
        root->child.erase(iter2);
        root->key.erase(iter1);
    } else if (r != v->child.size() - 1) {
        tempExternalNode = reinterpret_cast<InternalNode *>(root->child[r + 1]);
        for (int i = v->child.size()-1; i >= 0; i--) {
            tempExternalNode->key.insert(tempExternalNode->key.begin(), getMin(tempExternalNode->child[0]));
            tempExternalNode->child.insert(tempExternalNode->child.begin(), v->child[i]);
        }
        root->child.erase(iter2);
        root->key.erase(iter1);
    }
    //4.向上递归解决下溢
    solveDownflow(reinterpret_cast<InternalNode *>(root));
    return;

}

Keytype getMin(Node *root)
{
    if (root->isLeaf)
        return reinterpret_cast<ExternalNode *>(root)->data[0].first;
    return getMin(reinterpret_cast<InternalNode *>(root)->child[0]);
}
void BplusTree::levelOrder()
{
    queue<Node *> q;
    q.push(_root);
    int currentlineflag = 1;
    int nextlineflag = 0;
    int numbercount = 0;
    while (!q.empty())
    {
        Node *temp = q.front();
        q.pop();
        if (!temp->isLeaf)
        {
            InternalNode *tempInternal = reinterpret_cast<InternalNode *>(temp);
            cout << "| ";
            for (int i = 0; i < tempInternal->key.size(); i++)
                cout << tempInternal->key[i] << " ";
            cout << "| ";
            for (int i = 0; i < tempInternal->child.size(); i++)
                q.push(tempInternal->child[i]);
            numbercount++;
            nextlineflag += tempInternal->child.size();
            if (currentlineflag == numbercount)
            {
                cout << endl;
                currentlineflag = nextlineflag;
                nextlineflag = 0;
                numbercount = 0;
            }
        }
        else
        {
            ExternalNode *tempExternal = reinterpret_cast<ExternalNode *>(temp);
            cout << "| ";
            for (int i = 0; i < tempExternal->data.size(); i++)
                cout << tempExternal->data[i].first << " ";
            cout << " | ";
        }
    }
    cout << endl;
}

int main()
{
    BplusTree BTree;
    for (int i = 0; i < 18; i++)
        BTree.insert(make_pair(i, i * 2));
    BTree.insert(make_pair(20, 1));
    BTree.insert(make_pair(21, 1));
    BTree.insert(make_pair(22, 1));
    BTree.insert(make_pair(18, 1));
    cout << "insert success!" << endl;
    BTree.levelOrder();
    for (int i = 22; i >= 11; i--)
        BTree.remove(i);
    // BTree.remove(22);
    // BTree.remove(21);
    // BTree.remove(20);
    //BTree.remove(8);
    //BTree.remove(0);
    //BTree.remove(2);
    cout << "remove success!" << endl;
    BTree.levelOrder();
    cout << "success!" << endl;
}