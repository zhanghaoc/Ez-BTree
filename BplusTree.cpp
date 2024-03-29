﻿#include "BplusTree.h"
#include <queue>

//版本1：
bool BplusTree::search(const Keytype target)
{
    int time = 0;
    return search(target, time);
}

//重载版本2：
bool BplusTree::search(const Keytype target, int &time)
{
    InternalNode *p = reinterpret_cast<InternalNode *>(_root);
    while (!(p->isLeaf)) //当p一直为内部结点
    {
        int r = p->key.size() - 1;
        while (r >= 0 && p->key[r] > target)
            r--; //在p指向的结点的键值中找到不大于target的最大key, r代表秩rank, 即位置
        //若target不会比最小的key还小(此时r=-1)且r处的值与target相等

        p = reinterpret_cast<InternalNode *>(p->child[r + 1]);
        time += DELAY;
    }
    //下探到外部结点处了
    time += DELAY;
    ExternalNode *q = reinterpret_cast<ExternalNode *>(p);
    //找到data中键值不大于target的元素的位置
    for (int i = 0; i < q->data.size(); i++)
        if (target == q->data[i].first)
            return true;
    return false;
}

//版本1：
bool BplusTree::insert(const Datatype record)
{
    int time = 0;
    return insert(record, time);
}

//重载版本2：
bool BplusTree::insert(const Datatype record, int &time)
{
    //1、确认该元素不在B+树中
    if (search(record.first, time)) //如果找到了，说明不需要插入
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
    //得到的iter为所需要插入的位置的迭代器
    v->data.insert(iter, record);

    //3、检查是否上溢，若上溢进行第4步，否则返回
    if (v->data.size() <= _L) //没有上溢
        return true;

    //4、则开新的外部结点，将内容移过去
    int s = _L / 2; //中间元素的位置

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
    k = u->data[0].first;
    while (r >= 0 && q->key[r] > k)
        r--; //需要插入新键值的位置为r，需要插入新分支的位置为r+1
    r++;
    for (int i = 0; i < r; i++)
        iter2++;
    for (int i = 0; i < r + 1; i++)
        iter3++;
    q->key.insert(iter2, u->data[0].first);
    q->child.insert(iter3, u);

    solveOverflow(q);
    return true;
}

//解决内部结点上溢问题，使用递归
void BplusTree::solveOverflow(InternalNode *v)
{
    //1、判断是否上溢，此处为递归基
    if (v->child.size() <= _M)
        return;
    //2、创建新结点u，将中间元素之后的child和key移动过去
    InternalNode *u = new InternalNode();
    int s = (_M + 1) / 2;
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

//版本1：
bool BplusTree::remove(const Keytype target)
{
    int time = 0;
    return remove(target, time);
}

//重载版本2：
bool BplusTree::remove(const Keytype target, int &time)
{
    //1、确认该元素是否在B+树中
    if (!search(target, time)) //如果没找到，说明不需要删除
        return false;

    //2、找到该元素所在的位置并删除
    InternalNode *p = reinterpret_cast<InternalNode *>(_root);
    InternalNode *t = p;
    int r = p->key.size() - 1;
    while (!p->isLeaf)
    {
        t = p;
        r = p->key.size() - 1;
        while (r >= 0 && p->key[r] > target)
            r--; //在p指向的结点的键值中找到不大于target的最大key, r代表秩rank, 即位置
        r++;     //r = 0~child.size()
        p = reinterpret_cast<InternalNode *>(p->child[r]);
    }
    //执行完之后，当前结点为p，其双亲结点为t
    ExternalNode *v = reinterpret_cast<ExternalNode *>(p);

    vector<Datatype>::iterator iter = v->data.begin();
    for (iter; iter != v->data.end(); iter++)
        if (iter->first == target)
            break;
    v->data.erase(iter);

    //如果是该结点是根结点，则直接返回，因为根结点的存放的数据量没有下限
    if (v == _root)
        return true;
    //若target不在第一个元素, 为了保险更新一下t的键值
    if (r != 0)
        t->key[r - 1] = v->data[0].first;

    //3.判断叶节点元素下溢，若过少，进行第4步
    if (v->data.size() >= (_L + 1) / 2)
        return true; 

    //4.判断是否能从左右兄弟拿到元素，若无法拿到，进行第5步
    //r是数组下标
    ExternalNode *u = NULL;
    if (r != 0)//有左兄弟 
    {
        u = reinterpret_cast<ExternalNode *>(t->child[r - 1]);
        if (u->data.size() > (_L + 1) / 2)
        {
            v->data.insert(v->data.begin(), u->data[u->data.size() - 1]);
            u->data.pop_back();
            t->key[r - 1] = v->data[0].first;
            return true;
        }
    }
    if (r != t->child.size() - 1)//有右兄弟
    {
        u = reinterpret_cast<ExternalNode *>(t->child[r + 1]);
        if (u->data.size() > (_L + 1) / 2)
        {
            v->data.insert(v->data.end(), u->data[0]);
            u->data.erase(u->data.begin());
            t->key[r] = u->data[0].first;
            return true;
        }
    }
    //左右都借不成
    //5.与左兄弟或右兄弟合并，并删除相关结点
    vector<Keytype>::iterator iter1 = t->key.begin();
    vector<Node *>::iterator iter2 = t->child.begin();
    for (int i = 0; i < r; i++)
        iter1++, iter2++;
    if (r != 0)//与左兄弟合并
    {
        u = reinterpret_cast<ExternalNode *>(t->child[r - 1]);
        for (int i = 0; i < v->data.size(); i++)
            u->data.insert(u->data.end(), v->data[i]);
        iter1--;
        t->child.erase(iter2);
        t->key.erase(iter1);
    }
    else if (r != t->child.size() - 1) //与右兄弟合并
    {
        u = reinterpret_cast<ExternalNode *>(t->child[r + 1]);
        for (int i = v->data.size() - 1; i >= 0; i--)
            u->data.insert(u->data.begin(), v->data[i]);
        t->key[r] = u->data[0].first;
        t->child.erase(iter2);
        t->key.erase(iter1);
    }
    //6.递归解决双亲结点t的下溢
    solveUnderflow(reinterpret_cast<InternalNode *>(t)); //递归解决双亲结点的下溢
    return true;
}

void BplusTree::solveUnderflow(InternalNode *v)
{
    //1.检查结点是否下溢，此处为递归基    
    if (v->child.size() >= (_M + 1) / 2)
        return;

    //2.先查看是否能从相邻结点拿一个元素
    InternalNode *u = NULL;
    InternalNode *root = reinterpret_cast<InternalNode *>(v->parent);
    //v是_root的时候，检测child是否只剩一个元素，
    //若是，删去原来的root，用其child替代，这是树唯一减少高度的方式
    if (!root)
    {
        if (v->isLeaf) //如果v是根结点则不用管下溢
            return;
        if (v->key.size() == 0)
        {
            _root = v->child[0];
            _root->parent = NULL;            
        }
        return;
    }
    int r = 0;
    for (r = 0; r < root->child.size(); r++)
        if (root->child[r] == v)
            break;
    //先看左边是否有元素，同时判断个数是否符合要求，若符合，拿一个过来
    if (r != 0)
    {
        u = reinterpret_cast<InternalNode *>(root->child[r - 1]);
        if (u->child.size() > (_M + 1) / 2)
        {
            v->key.insert(v->key.begin(), getMin(v->child[0]));
            v->child[0]->parent = v;
            v->child.insert(v->child.begin(), u->child.back());
            u->key.pop_back();
            u->child.pop_back();            
            root->key[r - 1] = getMin(root->child[r]);
            return;
        }
    }
    //再看右边是否有元素，同时判断个数是否符合要求，若符合，拿一个过来
    if (r != root->child.size() - 1)
    {
        u = reinterpret_cast<InternalNode *>(root->child[r + 1]);
        if (u->child.size() > (_M + 1) / 2)
        {
            v->child.insert(v->child.end(), u->child[0]);
            v->child.back()->parent = v;
            v->key.insert(v->key.end(), getMin(v->child.back()));
            u->key.erase(u->key.begin());
            u->child.erase(u->child.begin());            
            root->key[r] = getMin(root->child[r + 1]);
            return;
        }
    }
    //3.此时相邻node元素个数均不符合要求，将其中一个相邻node合并
    //同时删去相关的key和child
    vector<Keytype>::iterator iter1 = root->key.begin();
    vector<Node *>::iterator iter2 = root->child.begin();
    for (int i = 0; i < r; i++)
        iter1++, iter2++;
    if (r != 0)
    {
        u = reinterpret_cast<InternalNode *>(root->child[r - 1]);
        for (int i = 0; i < v->child.size(); i++)
        {
            u->child.insert(u->child.end(), v->child[i]);
            u->key.insert(u->key.end(), getMin(u->child.back()));
            u->child.back()->parent = u;
        }
        iter1--;
        root->child.erase(iter2);
        root->key.erase(iter1);
    }
    else if (r != v->child.size() - 1)
    {
        u = reinterpret_cast<InternalNode *>(root->child[r + 1]);
        for (int i = v->child.size() - 1; i >= 0; i--)
        {
            u->key.insert(u->key.begin(), getMin(u->child[0]));
            u->child.insert(u->child.begin(), v->child[i]);
            u->child[0]->parent = u;
        }
        root->child.erase(iter2);
        root->key.erase(iter1);
    }
    //4.向上递归解决下溢
    solveUnderflow(reinterpret_cast<InternalNode *>(root));
    return;
}

Keytype BplusTree::getMin(Node *root)
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
    int linecnt = 0;
    bool first1 = true;
    bool first2 = true;
    cout << "__________________________________________" << endl;
    while (!q.empty())
    {
        if (first1)
        {
            //cout << "__________________________________________" << endl;
            cout << "The Nodes at level " << linecnt << endl;
            first1 = false;
        }
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
                linecnt++;
                cout << endl;
                cout << "The Nodes at level " << linecnt << endl;
                currentlineflag = nextlineflag;
                nextlineflag = 0;
                numbercount = 0;
            }
        }
        else
        {
            if (first2)
                cout << "......They are Leaves" << endl, first2 = false;
            ExternalNode *tempExternal = reinterpret_cast<ExternalNode *>(temp);
            cout << "| ";
            for (int i = 0; i < tempExternal->data.size(); i++)
                cout << "(" << tempExternal->data[i].first << " " << tempExternal->data[i].second << ") ";
            cout << " | ";
        }
    }
    cout << endl;
}
