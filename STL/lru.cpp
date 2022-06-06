// clang++ -g lru2.cpp
#include <list>
#include <unordered_map>
#include <iostream>
using namespace std;

typedef struct kvpair
{
   int key;
   int value;
} kvpair;
class LRUCache
{
    // store key/value of cache
    list<kvpair> dq;

    // store references of key in cache
    unordered_map<int, list<kvpair>::iterator> ma;
    int csize; //maximum capacity of cache

public:
    LRUCache(int); // constructor
    int get(int);
    void put(int, int);
    void display();
};

LRUCache::LRUCache(int n)
{
    csize = n;
}

/* Refers key x with in the LRU cache */
void LRUCache::put(int key, int value)
{
    kvpair p;
    // not present in cache
    if (ma.find(key) == ma.end())
    {
        // cache is full
        if (dq.size() == csize)
        {
            //delete least recently used element
            kvpair last = dq.back();
            dq.pop_back();
            ma.erase(last.key);
        }
    }

    // present in cache
    else {
        //p = *ma[key];
        dq.erase(ma[key]);
    }
    // update reference
    p.key= key;
    p.value= value;
    dq.push_front(p);
    ma[key] = dq.begin();
}

int LRUCache::get(int key)
{
   unordered_map<int, list<kvpair>::iterator>::iterator mp;
   if ((mp =ma.find(key)) == ma.end())
   {
       return -911;
   }
   kvpair kvp = *(mp->second);
   dq.erase(mp->second);
   dq.push_front(kvp);
   ma[key] = dq.begin();
   return kvp.value;
}



// display contents of cache
void LRUCache::display()
{
    for (list<kvpair>::iterator it = dq.begin(); it != dq.end();
                                        it++)
        std::cout << (it)->key << "=" << it->value << " ";

    std::cout << std::endl;
}

// Driver program to test above functions
int main()
{
    LRUCache ca(4);

    ca.put(1,10);
    ca.put(2,20);
    ca.put(3,30);
    cout << "gettign 1 " << ca.get(1) << endl;
    ca.put(4,40);
    ca.put(5,50);
    ca.display();
    cout << "-----" << endl;
    ca.put(6,60);
    cout << "gettign 1 " << ca.get(1) << endl;
    cout << "gettign 2 " << ca.get(2) << endl;
    ca.put(6,61);
    ca.put(7,70);
    ca.display();

    return 0;
}
