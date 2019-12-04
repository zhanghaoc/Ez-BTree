#include"BplusTree.cpp"
#include<cstdlib>
int main()
{
    BplusTree Tree;
    bool run = true; //不修改成false则一直运行
    while(run)
    {
        cout << "select the operations you want:" << endl;
        cout << "1.insert a element" << endl; //插入元素，并输出所用时间
        cout << "2.delete a element" << endl; //删除元素，并输出所用时间
        cout << "3.search a element" << endl; //查找元素，并输出所用时间
        cout << "4.levelOrder travel this Tree" << endl; //层次遍历
        cout << "5.test the system with randomly inserting elements" << endl; 
        //随机插入测试，键值取值[0, 1000]，数值取值[0, 10000]
        cout << "6.test the system with randomly deleting elements" << endl; //随机删除测试
        //随机删除测试，键值取值[0, 1000]
        cout << "7.exit" << endl; //退出系统
        cout << "Your input: ";
        string op;
        cin >> op;
        if(op.size() != 1) //检查输入是否合法
        {
            cout << "invalid input!" << endl;
            continue;
        }
        Keytype key;
        Valuetype value;
        Datatype data;
        int N;
        int cnt = 0;
        int time = 0;
        switch(op[0])
        {            
            case '1':
                cout << "input the key and the value of this element" << endl; 
                cout << "key: ";                                           
                cin >> key;                
                cout << "value: ";
                cin >> value;
                data.first = key;
                data.second = value;
                if(Tree.insert(data, time))
                {
                    cout << "Insert success!" << endl;
                    cout << "Time used: " << time << "ms" << endl;
                }                    
                else    cout << "insert fail!" << endl;
                break;
            case '2':
                cout << "input the key" << endl;                               
                cin >> key;                
                if(Tree.remove(key, time))
                {
                    cout << "remove success!" << endl;
                    cout << "Time used: " << time << "ms" << endl;
                }
                    
                else    cout << "remove fail!" << endl;
                break;
            case '3':
                cout << "input the key" << endl;                                
                cin >> key;                
                if(Tree.search(key, time))
                {
                    cout << "element with this key exists" << endl;
                    cout << "Time used: " << time << "ms" << endl;
                }                    
                else    cout << "element with this key doesn't exist" << endl;
                break;
            case '4':
                cout << "Here is the levelOrder traversal of this B+Tree." << endl;
                Tree.levelOrder();
                break;
            case '5':
                cout << "input the number of elements you want to insert randomly" << endl;
                cout << "number: " << endl;
                cin >> N;                
                for(int i=0;i<N;i++)
                {
                    key = rand() % 1000;
                    value = rand() % 10000;
                    cout << "key: " << key << " value: " << value << endl;
                    if(Tree.insert(make_pair(key, value)))
                        cnt++;                
                }
                cout << "the number of successful insertion is " << cnt << endl;
                cout << "now the Tree looks like this" << endl;
                Tree.levelOrder();
                break;
            case '6':
                cout << "input the number of elements you want to delete randomly" << endl;
                cout << "number: " << endl;
                cin >> N;                
                for(int i=0;i<N;i++)
                {
                    key = rand() % 1000;                    
                    cout << "key: " << key << endl;
                    if(Tree.remove(key))
                        cnt++;                
                }
                cout << "the number of successful deletion is " << cnt << endl;
                cout << "now the Tree looks like this" << endl;
                Tree.levelOrder();
                break;
            case '7':
                run = false;
                break;
            default:
                cout << "invalid input!" << endl;
        }        
        cout << "______________________________________________________" << endl;
    }
}