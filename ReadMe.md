# Concurrent LRU Cache System

We will keep Concurrent HashMap and a singly linked list. The size of cache is fixed.

#### Node structure in singly linked list
each node will have pointer to next node and a value parameter.

##### The order of nodes in linked list represents value access order -> from recently used to least recently used.

##### For key-value pair, the HashMap will store key and corresponding values and the linked list holds keys only.


## Algorithm
When a new key-value is accessed, we check if key is present in map or not.
 + If key is present in map,then it is cache hit. We return value corresponding to given key from map and we bring node with this key to front of linked list
 + Otherwise it is cache miss. we need to bring this new key-value into the cache system. 
   + If no of elements in the cache system is less than size of system, then create node with given key as parameter and insert it in front of linked list and store key-value in map
   + Otherwise remove tail of linked list. create new node with new value and insert it in front of linked list and store key-value pair in map 





### API

##### LRUCache(int given_size) -> creates cache system with maximum possible no of elements in the cache system= given_size

