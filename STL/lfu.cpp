// C++ program for LFU cache implementation
#include <iostream>
#include <unordered_map>
#include <vector>
#include <algorithm>

using namespace std;

#if 0
// Generic function to swap two pairs
void swap(pair<int, int>& a, pair<int, int>& b)
{
   pair<int, int> temp = a;
   a = b;
   b = temp;
}
#endif

// Returns the index of the parent node
inline int parent(int i)
{
   return (i - 1) / 2;
}

// Returns the index of the left child node
inline int left(int i)
{
   return 2 * i + 1;
}

// Returns the index of the right child node
inline int right(int i)
{
   return 2 * i + 2;
}

// Self made heap tp Rearranges
// the nodes in order to maintain the heap property
void heapify(vector<pair<int, int> >& v,
         unordered_map<int, int>& m, int i, int n)
{
   int l = left(i), r = right(i), minim;
   if (l < n)
      minim = ((v[i].second < v[l].second) ? i : l);
   else
      minim = i;
   if (r < n)
      minim = ((v[minim].second < v[r].second) ? minim : r);
   if (minim != i) {
      m[v[minim].first] = i;
      m[v[i].first] = minim;
      swap(v[minim], v[i]);
      heapify(v, m, minim, n);
   }
}

// Function to Increment the frequency
// of a node and rearranges the heap
void increment(vector<pair<int, int> >& v,
         unordered_map<int, int>& m, int i, int n)
{
   ++v[i].second;
   heapify(v, m, i, n);
}

// Function to Insert a new node in the heap
void insert(vector<pair<int, int> >& v,
         unordered_map<int, int>& m, int value, int& n)
{
   
   if (n == v.size()) {
      m.erase(v[0].first);
      cout << "Cache block " << v[0].first
                     << " removed.\n";
      v[0] = v[--n];
      heapify(v, m, 0, n);
   }
   v[n++] = make_pair(value, 1);
   m.insert(make_pair(value, n - 1));
   int i = n - 1;

   // Insert a node in the heap by swapping elements
   while (i && v[parent(i)].second > v[i].second) {
      m[v[i].first] = parent(i);
      m[v[parent(i)].first] = i;
      swap(v[i], v[parent(i)]);
      i = parent(i);
   }
   cout << "Cache block " << value << " inserted.\n";
}

class lfu {
private:
    int cache_max_size;
    vector<pair<int, int> > *cache;
    unordered_map<int, int> indices;
public:
    lfu(int size) : cache_max_size(size) {
       cache = new vector<pair<int, int> > (size);
    }
    void refer(int value, int &cache_size) {
   if (indices.find(value) == indices.end())
      insert(*cache, indices, value, cache_size);
   else
      increment(*cache, indices, (indices)[value], cache_size);
    }
    void dump() {
      for (auto it=(*cache).begin(); it<(*cache).end(); it++)
         cout << it->first << ":" << it->second << " ";
      cout << endl;
    }
};

#if 1
// Function to refer to the block value in the cache
void refer(vector<pair<int, int> >& cache, unordered_map<int,
               int>& indices, int value, int& cache_size)
{
   if (indices.find(value) == indices.end())
      insert(cache, indices, value, cache_size);
   else
      increment(cache, indices, indices[value], cache_size);
}
#endif

// Driver Code
int main()
{
#if 1
   int cache_max_size = 4, cache_size = 0;
   vector<pair<int, int> > cache(cache_max_size);
   unordered_map<int, int> indices;
   refer(cache, indices, 1, cache_size);
   refer(cache, indices, 2, cache_size);
   refer(cache, indices, 1, cache_size);
   refer(cache, indices, 3, cache_size);
   refer(cache, indices, 2, cache_size);
   refer(cache, indices, 4, cache_size);
   refer(cache, indices, 1, cache_size);
   refer(cache, indices, 5, cache_size);
   refer(cache, indices, 5, cache_size);
   refer(cache, indices, 6, cache_size);
   refer(cache, indices, 1, cache_size);
   refer(cache, indices, 4, cache_size);
   for (auto it=cache.begin(); it<cache.end(); it++)
      cout << it->first << ":" << it->second << " ";
   cout << endl;
#endif
#if 1
   cache_size = 0;
   lfu LFU(4);
   LFU.refer(1, cache_size);
   LFU.refer(2, cache_size);
   LFU.refer(1, cache_size);
   LFU.refer(3, cache_size);
   LFU.refer(2, cache_size);
   LFU.refer(4, cache_size);
   LFU.refer(5, cache_size);
   LFU.refer(6, cache_size);
   LFU.refer(1, cache_size);
   LFU.dump();
#endif
   return 0;
}

