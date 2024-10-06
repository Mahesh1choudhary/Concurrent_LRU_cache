# Concurrent LRU Cache System
First we consider how we can implement single threaded LRU cache. We use doubly linked list for storing values in order representing usage order (most recently used will be head of list) and a map storing key-reference to node in linked list. For removing a node from linked list, we can use map to directly move to node respective to given key, and since list is doubly linked, we can get previous and next node and can remove current node. But we cannot use key-reference to node kind strategy in multithreaded implementation, I will explain reasons later.


### Implementation and structure
We use thread-safe hash Map and thread safe singly linked list. Map stores key-value and linked list stores keys in order of usage.

We continue by first describing implementation and api details of singly linked list and Hash Map.

##  Thread-safe singly linked list
In this, each node will have pointer to next node, a parameter stored and a mutex.

Since list is singly linked, so we can move from beginning towards end only, not the other way around. This is essential to avoid deadlocks. For example, for removing a node, we need previous node and current node. We need to lock mutex of the corresponding node that we want to access as the environment is multithreaded. So before removing a node, we need to take lock in some order, either previous -> current or current->previous. We choose first case.
We can use doubly linked list and restrict movement in one direction only. But then using doubly linked list has no benefits over singly linked list, and it will be worse as we have to tack lock on previous, current and next node, instead of just previous and current.

For removing a node in this singly linked list, we have to start from head and move next to find required node while also keeping lock on previous and current node. We can't use doubly linked list and get previous node, as in multithreaded environment, it can lead to deadlock and other race conditions.
Example-> consider four nodes   a-> <- b -> <- c -> <- d
if thread1 tries to delete b and thread2 tries to delete c. We need to take lock on previous, current and next node to remove current node. Also, if we want to get previous node through current node, then we have to first take lock on current node as if we don't take it, it can be modified in between by some other thread(like two threads tries to remove one node,and one of the thread tries to access it even when it is already removed by other thread)
 this scenario can arise: thread1 has taken lock on a and b, thread2 on c. Now thread1 want lock on c, but c wants lock on b, so there is deadlock.
Hence we cannot use key-reference to node in hash map for multithreaded implementation

In implementation, we kept a dummy head to make implementation easy. By doing so, we don't have to consider cases where head changes( when head is removed). So we made a constant head

#### Linked list API

  + ThreadSafeSinglyLinkedList() -> to initialise the concurrent linked list structure.

  + void add_front(const Value &value)  -> add a new node at the start of linked list with given value.  It return nothing
  This method first takes lock on dummy head and then add new node next to dummy head node.


  + bool erase(Value value)   ->   If node with given value exist, this method will remove first occurrence of this value and returns true. If no such node exists, then this will return false. For removing a node, we have to take lock on previous node and current node.


  + void erase_tail(Value &val) ->  If last element exists, then function return true and copies value stored in last node into passed parameter, otherwise returns false.



## Thread safe Concurrent Hash Map
  Lets first define certain terms and data structures that are used in this approach.
    1. HashNode-> A node in a linkedlist with key-value pair and next pointer
    2. HashBucket -> linkedlist of HashNodes
  
  We will create an array of HashBuckets and will store all the key-value pairs in corresponding Hash Buckets as HashNode. The size of the array has great impact on efficiency. If it is too small, then operations like insert,erase, find, etc perform same as on linkedlist; If too large , there will be empty spaces in array, so extra unnecessary memory usage occurs.
  The index of array to which given key-value belongs depends on the hash value of key and size of array selected. the convention we are using is
  >array index = hash_value(key)%array_size;

  Hash Function should also be good enough for better distribution of key-value pairs.

  ###### For locking, we used shared_mutex which allows multiple reads at the same time , but single write at a time. The reason for choosing this lock is that in cache system, the cache miss is very low , so there will be very less number of writes as compared to reads.

  #### Hash Map API


  + ConcurrentHashMap(HashSize) - Creates an ConcurrentHashMap with HashSize no of buckets

  + find(key, value) - If key is present in map , then corresponding value is copied into provided value parameter and function return true, otherwise it return false

  + insert(key, value) - Inserts new key-value in map if key is not present in map, otherwise updates value of the key in map 

  + erase(key) - removes entry corresponding to given key in the map

  + clear() - removes all entries from map
   


#### In hash map , we store key- value_node pair. value_node stores value and a boolean which represents whether value is updated by cpu. When removing elements from cache, we will write back only those key-value pairs in which value is updated, others will be neglected

### LRU cache system API

+ LRUCacheSystem(given_size) - initialises cache system with parameter given_size = maximum possible no of elements in the cache sysem

+ bool isEmpty() - return true if cache is empty, otherwie false

+ void clear() - clears cache

+ int size() - returns current number of elements in cache

+ void update(Key, Value) - updates value of given key with new value.
    + if key is present in cache, it is cache hit
        + if given value is same as present, then just update linked list( bring given key in front)
        + otherwise, mark it as updated, insert updated value into map and update linked list by bringing key in front
    + otherwise mark value as updated and insert into map, update linked list. Remove keys if necessary. If removing keys, write back those keys whose values are updated.

+ Value get_Value(Key) - returns value corresponding to given key.
    + If key is present in map,then it is cache hit. We return value corresponding to given key from map and we bring node with this key to front of linked list
   + Otherwise it is cache miss. we need to bring this new key-value into the cache system. Read value corresponding to given key from memory
     + If no of elements in the cache system is less than size of system, then create node with given key as parameter and insert it in front of linked list and store key-value_node in map
     + Otherwise remove tail of linked list. create new node with new key and insert it in front of linked list and store key-value_node pair in map. Also if elements that is removed is updated, then write back to main memory

