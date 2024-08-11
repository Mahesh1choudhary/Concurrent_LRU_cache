#ifndef HASH_BUCKET_H_
#define HASH_BUCKET_H

#include "HashNode.h"
#include <shared_mutex>
#include <mutex>

namespace CHML{

    // Bucket implementation as singly linked list of HashNodes
    template<typename Key, typename Value> class HashBucket
    {
        public:
            HashBucket()
            {

            }
            ~HashBucket()
            {
                clear();
            }

            // function for searching an entry in bucket with given key
            // if value is found in this bucket then function returns true and copies value to passed parameter value_, otherwise returns false
            bool find(const Key &key_, Value &value_) const{
                std::shared_lock lock(mutex_);
                HashNode<Key,Value> *node =head;
                while(node!=nullptr){
                    if(node->getKey() ==key_){
                        value_= node->getValue();
                        return true;
                    }
                    node= node->next;
                }
                return false;
            }

            // function to insert into bucket
            // if key is not present in bucket, then insert new node with given key-value pair , otherwise update value
            void insert(const Key &key_,const Value &value_){
                std::unique_lock lock(mutex_);
                HashNode<Key,Value> *prev=nullptr;
                HashNode<Key,Value> *node =head;

                while(node!=nullptr && node->getKey()!=key_){
                    prev=node;
                    node = node->next;
                }
                if(node==nullptr){
                    if(head==nullptr){
                        head=new HashNode<Key,Value>(key_,value_);
                    }
                    else{
                        prev->next = new HashNode<Key,Value>(key_,value_);
                    }
                }
                else{
                    node->setValue(value_);
                }
            }

            // function to remove an entry from bucket, if found
            void erase(const Key &key_){
                std::unique_lock lock(mutex_);
                HashNode<Key,Value> *prev=nullptr;
                HashNode<Key,Value> *node =head;

                while(node!=nullptr && node->getKey()!=key_){
                    prev=node;
                    node = node->next;
                }
                if(node==nullptr){
                    return;
                }
                else{
                    if(head==node){
                        head=node->next;
                    }
                    else{
                        prev->next = node->next;
                    }
                    delete node;
                }
            }

            void clear(){
                std::unique_lock lock(mutex_);
                HashNode<Key,Value> *prev= nullptr;
                HashNode<Key,Value> *node=head;
                while(node!=nullptr){
                    prev=node;
                    node= node->next;
                    delete prev;
                }
                head=nullptr;
            }

        private:
            HashNode<Key,Value> *head = nullptr;    // head of bucket
            mutable std::shared_mutex mutex_; // mutex for this bucket
    };
}
#endif