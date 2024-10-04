#ifndef HASH_MAP_H_
#define HASH_MAP_H_

#include "HashBucket.h"
#include <mutex>
#include <cstdint>
#include <functional>

constexpr size_t DEFAULT_HASH_SIZE = 1031; // a prime no for better distribution
namespace CHML{ // Concurrent Hash Map Library

    // we have to provide key type , value type and Hash function(if key type is user defined, otherwise we will use deafult hash functions)
    // If hash size is not provided, then default size of 1031 is used

    /*
    Hash Table is implemented as an array of HashBuckets of given/default size. All HashBuckets will be created during cosntruction of map
    Each HashBucket has its own associated lock, so multiple threads can write simultaneously different buckets
    */

    template<typename Key, typename Value,
    typename HashFunc= std::hash<Key>> class ConcurrentHashMap{

        public:
        ConcurrentHashMap(size_t HashSize_= DEFAULT_HASH_SIZE): HashSize(HashSize_){
            HashTable= new HashBucket<Key,Value>[HashSize];
        }

        ~ConcurrentHashMap(){
            delete[] HashTable;
        }

        // deleting copy and move constructors to maintain thread safety
        ConcurrentHashMap(const ConcurrentHashMap&)= delete;
        ConcurrentHashMap(ConcurrentHashMap&&)= delete;
        ConcurrentHashMap operator=(const ConcurrentHashMap&)= delete;
        ConcurrentHashMap operator=(ConcurrentHashMap&&)= delete;

        // function to find an entry in the HashMap with given key
        //If key is found, then corresponding value is copied to passed parameter and fucntion returns true, otherwise function return false
        // Here hasfunction should be good enough. For example if hasfunction hashes all values 1 to 10, then there is no benefit of keeping hashsize to be more than 10
        bool find(const Key &key, Value &value) const{
            size_t hashvalue= hashfunction(key)%HashSize;
            return HashTable[hashvalue].find(key,value);
        }

        // function to insert into Hash Map
        // If key already exsits , then update the value, otherwise insert a new node in corresponding bucket
        void insert(const Key &key, const Value &value){
            size_t hashvalue= hashfunction(key)%HashSize;
            HashTable[hashvalue].insert(key,value);
        }

        // function to remove an entry from the hashtable if present
        void erase(const Key &key){
            size_t hashvalue= hashfunction(key)%HashSize;
            HashTable[hashvalue].erase(key);
        }

        // function to clean up the hash map
        void clear(){
            for(size_t i=0;i<HashSize;i++){
                HashTable[i].clear();
            }
        }

        private:
            HashBucket<Key, Value> *HashTable;
            HashFunc hashfunction;
            const size_t HashSize;

    };
}

#endif