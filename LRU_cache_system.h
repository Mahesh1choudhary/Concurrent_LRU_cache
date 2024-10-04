#ifndef LRU_Cache_System
#define LRU_Cache_System

#include "./helper/ConcurrentHashMap.h"
#include "./helper/ThreadSafeSinglyLinkedList.h"
#include "./helper/MainMemory.h"
#include <mutex>

namespace LRU{

    std::mutex y;   //

    template<typename Value> class Value_Node{
        public:
            Value value;
            bool updated=false;
            Value_Node(){
                updated=false;
            }
            Value_Node(Value value_){
                value= value_;
                updated=false;      // to write back only those cache key which are updated
            }
    };

    template<typename Key, typename Value> class LRUCacheSystem{

        public:
            // We are assuming that a single thread will first initialises the cache system, and then multiple thread will work with it. 
            //So we don't have to worry about multiple initialisation, taking lock on curr_size in this initialisation, etc
            LRUCacheSystem(int given_size=30): max_size(given_size){
                curr_size=0;
            }

            // deleting copy and move constructors to maintain thread safety
            LRUCacheSystem(const LRUCacheSystem&)= delete;
            LRUCacheSystem(LRUCacheSystem&&)=delete;
            LRUCacheSystem& operator=(const LRUCacheSystem&)=delete;
            LRUCacheSystem& operator=(LRUCacheSystem&&)=delete;
            





            // function to check if there are any elements in cache system
            bool isEmpty(){
                std::unique_lock<std::mutex> lk(curr_mt);
                return curr_size==0;
            }

            // function to clear the complet cache - while clearing, adding and removing of elements is possible, but it should be avoided, otherwise clearing does not make sense
            void clear(){
                mp.clear();
                lst.clear();

                std::unique_lock<std::mutex> lk(curr_mt);
                curr_size==0;
            }
            
            // function to return current no of elements in the cache system
            int size(){
                std::unique_lock<std::mutex> lk(curr_mt);
                return curr_size;
            }

            // if some computation is done on values, then values need to be updated in cache system.
            //function to update value in cache system. Here we consider update also as a cache access
            void update(Key key_, Value value_){

                Value_Node<Value> val;
                if(mp.find(key_, val)){
                    // if values ( value in cache and new value to be put) are not different, then we do not consider it as a cache access, as this will be unnecessary
                    if(val.value ==value_) return;

                    // if values are different, then it will be considered as cache access and we update cache
                    val.value= value_;

                    mp.insert(key_,val);
                    
                    // if key is found in linkedlist , bring this at start of linkedlist, and update value of key if val!=value_
                    if(lst.erase(key_)){
                        /*
                        only if a thread erase node with given key, only this will add it in front.
                        why -> if two threads access the same key and one of the thread already removed the key from linkedlist, then it will add the key in front
                        If other thread tries to remove same key before first thread insert it in front, then this key is not present in linkedlist, so other thread will not be able to 
                        remove it, so will not add in front.
                        */
                        lst.add_front(key_);
                    }
                    
                }
                else{
                    // it will be considered as cache access
                    // if key is not found, then add new node in linkedlist at front and put key in map, Also delete tail from linkedlist if necessary
                    
                    val.updated=true;
                    val.value= value_;

                    mp.insert(key_,val);
                    lst.add_front(key_);

                    std::unique_lock<std::mutex> lk(curr_mt);
                    curr_size++;
                    lk.unlock();

                    if(curr_size > max_size){
                        lst.erase_tail(key_); // removes tail and copies its value into key_ variable
                        /*
                        IMPORTANT-> if value corresponding to this key is updated, then write back to memory
                        */
                        
                        // for testing only, otherwise write your logic to write back to main memory
                        mp.find(key_,val);  // get the val corresponding to key_

                        //
                        Value x;
                        memory.find_value(key_,x);
                        std::unique_lock<std::mutex> l(y);
                        std::cout<<"update , key= "<<key_<<", is_updated= "<<val.updated<<", value= "<<val.value<<", memory_value= "<<x<<std::endl;
                        l.unlock();
                        //



                        if(val.updated){
                            memory.write_back_to_memory(key_, val.value);
                            l.lock();   //
                            std::cout<<"here"<<std::endl;  //
                            l.unlock();     //
                        }



                        mp.erase(key_);  // remove key from map

                        lk.lock();
                        curr_size--;
                        lk.unlock();
                    }
                }
            }



            /* If key exists in cache system, then it is cache hit and we return correspnding value and update cache system accordinly.
                otherwise, it is a cache miss and we need to bring this key with value into cache

                if key is already in system, then bring it to start of linked list and update value parameter if needed
            If key is not present in map, then node with given value is inserted in front of linked list and key is inserted into map with given value.
            The node at tail is removed based on whether no of elements is less than maxsize or not
            */
            Value get_Value(Key key_){
                Value_Node<Value> val;
                if(mp.find(key_,val)){
                    /*
                    If key exists in cache system, bring it in front of linkedlist, fetch value from cache and return it;
                    */
                    if(lst.erase(key_)){
                            /*
                            only if a thread erase node with given key, only this will add it in front.
                            why -> if two threads access the same key and one of the thread already removed the key from linkedlist, then it will add the key in front
                            If other thread tries to remove same key before first thread insert it in front, then this key is not present in linkedlist, so other thread will not be able to 
                            remove it, so will not add in front.
                            */
                        lst.add_front(key_);
                    }
                }
                else{
                    /*
                    IMPORTANT -> if key does not exists in cahce, bring its value from main memory and put it in cache accordingly.
                    */


                    val.value=memory.read_from_memory(key_);// for testing only, otherwise use your own logic
                    val.updated=false;


                    mp.insert(key_,val);
                    lst.add_front(key_);

                    std::unique_lock<std::mutex> lk(curr_mt);
                    curr_size++;
                    lk.unlock();

                    Value_Node<Value> newval;
                    if(curr_size > max_size){
                        lst.erase_tail(key_); // removes tail and copies its corresponding key into key_ variable

                        /*
                        important-> if value corresponding to this key is updated, then write back to main memory before deleting from map, otherwise delete directly
                        */
                        mp.find(key_,newval);  // get the value_node corresponding to key_

                        //
                        Value x;
                        memory.find_value(key_,x);
                        std::unique_lock<std::mutex> l(y);
                        std::cout<<"getvalue , key= "<<key_<<", is_updated= "<<newval.updated<<", value= "<<newval.value<<", memory_value= "<<x<<std::endl;
                        l.unlock();
                        //


                        if(newval.updated){
                            memory.write_back_to_memory(key_, newval.value);
                            l.lock();                   //
                            std::cout<<"here"<<std::endl; //
                            l.unlock();     //
                        }

                        mp.erase(key_);  // remove key from map
                        lk.lock();
                        curr_size--;
                        lk.unlock();
                    }

                }
                return val.value;
            }


            // for testing only
            void display_list(){
                lst.display_list();
            }

            // for testing only- to check whether element exists in map or not , if yes, return value
            bool find_in_map(Key key_, Value &value_){
                Value_Node<Value> val;
                bool ans=mp.find(key_,val);
                value_= val.value;
                return ans;
            }


            MM::Main_Memory<Key,Value> memory;  // for testing only

        private:
            int max_size, curr_size;  // maximum possible no elements in cache system or size of cach system,  and current no of elements in the cache system
            std::mutex curr_mt; // mutex for curr_size
            CHML::ConcurrentHashMap<Key,Value_Node<Value>> mp;    // concurrent map
            CLL::ThreadSafeSinglyLinkedList<Key> lst;      // concurrent singly linked list
    };


}


#endif