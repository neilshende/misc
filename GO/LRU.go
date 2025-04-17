package main

import "container/list"

// LRUCache represents a least recently used (LRU) cache.
type LRUCache struct {
	capacity  int
	cache     map[interface{}]*list.Element
	ll        *list.List
}

// entry is the value stored in the linked list.
type entry struct {
	key   interface{}
	value interface{}
}

// Constructor for LRUCache.
func NewLRUCache(capacity int) *LRUCache {
	return &LRUCache{
		capacity:  capacity,
		cache:     make(map[interface{}]*list.Element),
		ll:        list.New(),
	}
}

// Get retrieves a value from the cache.
func (lc *LRUCache) Get(key interface{}) interface{} {
	if elem, ok := lc.cache[key]; ok {
		// Move the accessed element to the front of the list (most recently used).
		lc.ll.MoveToFront(elem)
		return elem.Value.(*entry).value
	}
	return nil // Key not found
}

// Put adds or updates a key-value pair in the cache.
func (lc *LRUCache) Put(key, value interface{}) {
	if elem, ok := lc.cache[key]; ok {
		// Key exists, update the value and move to the front.
		elem.Value.(*entry).value = value
		lc.ll.MoveToFront(elem)
	} else {
		// Key doesn't exist, create a new entry.
		newElem := lc.ll.PushFront(&entry{key: key, value: value})
		lc.cache[key] = newElem

		// Check if the cache has exceeded its capacity.
		if lc.ll.Len() > lc.capacity {
			// Remove the least recently used element (at the back of the list).
			oldElem := lc.ll.Remove(lc.ll.Back())
			delete(lc.cache, oldElem.(*entry).key)
		}
	}
}

// Len returns the current number of items in the cache.
func (lc *LRUCache) Len() int {
	return lc.ll.Len()
}

// Capacity returns the maximum capacity of the cache.
func (lc *LRUCache) Capacity() int {
	return lc.capacity
}

func main() {
	cache := NewLRUCache(3)

	cache.Put(1, "one")
	cache.Put(2, "two")
	cache.Put(3, "three")
	fmt.Printf("Cache size: %d\n", cache.Len()) // Output: Cache size: 3

	fmt.Printf("Get 1: %v\n", cache.Get(1))     // Output: Get 1: one
	fmt.Printf("Get 4: %v\n", cache.Get(4))     // Output: Get 4: <nil>

	cache.Put(4, "four") // This will evict the least recently used (2)
	fmt.Printf("Cache size after put(4): %d\n", cache.Len()) // Output: Cache size after put(4): 3
	fmt.Printf("Get 2: %v\n", cache.Get(2))     // Output: Get 2: <nil>

	cache.Put(2, "new_two") // This will evict the least recently used (3)
	fmt.Printf("Get 2: %v\n", cache.Get(2))     // Output: Get 2: new_two
	fmt.Printf("Get 3: %v\n", cache.Get(3))     // Output: Get 3: <nil>
}