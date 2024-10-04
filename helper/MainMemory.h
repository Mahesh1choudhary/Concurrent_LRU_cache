#ifndef HASH_Main_Memory
#define HASH_Main_Memory

#include <mutex>
#include <iostream>
#include <unordered_map>

//this is just for testing LRU_cache_system, so we are not optimising this
namespace MM{ // Main Memory
    template<typename Key, typename Value> class Main_Memory{
        public:

            Value read_from_memory(Key key_){
                std::unique_lock<std::mutex> lk(mt);
                return mp[key_];
            }

            void write_back_to_memory(Key key_, Value value_){
                std::unique_lock<std::mutex> lk(mt);
                mp[key_]=value_;
            }

            void insert(Key key_, Value value_){
                std::unique_lock<std::mutex> lk(mt);
                mp[key_]=value_;
            }

            bool find_value(Key key_, Value& value_){
                std::unique_lock<std::mutex> lk(mt);
                if(mp.count(key_)){
                    value_=mp[key_];
                    return true;
                }
                return false;
            }
        private:
            std::unordered_map<int,int> mp;
            std::mutex mt;  // to lock entire map
    };

}

#endif