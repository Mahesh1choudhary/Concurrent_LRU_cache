#ifndef HASH_Linked_List
#define HASH_Linked_List

#include <mutex>
#include <iostream>

namespace CLL{  // Concurrent LinkedList Library
    // simple structure to represent node in the list
    template<class Value> class Node{
        public:
            Node *next= nullptr;
            mutable std::mutex mutex_; 

            Node(){

            }

            Node(Value value_):value(value_){
            }

            Node(const Node&) =delete;
            Node(Node&&) = delete;
            Node& operator=(const Node&) =delete;
            Node& operator=(Node&&)= delete;


            ~Node(){
                next=nullptr;
            }

            const Value &getValue() const
            {
                return value;
            }
        private:
            Value value;
            
    };


    template<class Value=int> class ThreadSafeSinglyLinkedList{
        public:
            ThreadSafeSinglyLinkedList(){
                head= new Node<Value>(0);  // extra node
            }
                
            ~ThreadSafeSinglyLinkedList(){
                std::unique_lock<std::mutex> lock_curr(head->mutex_);
                Node<Value>* curr=head;
                while(curr->next !=NULL){
                    std::unique_lock<std::mutex> lock(head->next->mutex_);
                    head=head->next;
                    delete curr;
                }
                
            }

            // deleting copy and move constructors to maintain thread safety
            ThreadSafeSinglyLinkedList(const ThreadSafeSinglyLinkedList&)= delete;
            ThreadSafeSinglyLinkedList(ThreadSafeSinglyLinkedList&&)=delete;
            ThreadSafeSinglyLinkedList& operator=(const ThreadSafeSinglyLinkedList&)=delete;
            ThreadSafeSinglyLinkedList& operator=(ThreadSafeSinglyLinkedList&&)=delete;


            // function to insert node at the front of linkedlist
            void add_front(const Value &value){
                std::unique_lock<std::mutex> lock_head(head->mutex_);
                Node<Value>* new_node= new Node<Value>(value);
                
                new_node->next= head->next;
                head->next= new_node;
            }

            // function to erase given value from linkedlist. If value is found in list, then node is removed and function returns true, otherwise false;
            bool erase(Value value){
                std::unique_lock<std::mutex> lock_prev(head->mutex_);
                Node<Value>* prev= head;
                if(prev->next == nullptr){   // empty list
                    return false;
                }

                std::unique_lock<std::mutex> lock_curr(prev->next->mutex_);
                Node<Value>* curr= prev->next;
                
                while(curr->next!=nullptr && curr->getValue()!=value){
                    lock_prev.unlock();
                    prev= curr;
                    lock_prev.swap(lock_curr); // transferig lock ownership from curr to prev
                    
                    lock_curr=std::unique_lock<std::mutex>(prev->next->mutex_);
                    curr= prev->next;
                }

                // value is present in linked list
                if(curr->getValue()==value){
                    prev->next = curr->next;
                    lock_curr.unlock();
                    
                    delete curr;
                    return true;
                }
                return false;
            }

            /* function to remove last element from list if exists. If tail elements exists, then function returns true and copies value stored into passed parameter
            otherwise returns false;
            */
            bool erase_tail(Value &val){
                Node<Value> *prev=nullptr, *curr=nullptr;
                std::unique_lock<std::mutex> lock_prev(head->mutex_);
                prev= head;
                if(prev->next== nullptr){   // empty list
                    return false;
                }

                std::unique_lock<std::mutex> lock_curr(prev->next->mutex_);
                curr= prev->next;
                
                while(curr->next!= nullptr){
                    lock_prev.unlock();
                    prev= curr;
                    lock_prev.swap(lock_curr); // transferig lock ownership from curr to prev
                    
                    lock_curr=std::unique_lock<std::mutex>(prev->next->mutex_);
                    curr= prev->next;
                }

                prev->next = curr->next;
                lock_curr.unlock();
                val= curr->getValue();
                delete curr;
                return true;
            }


            // function to delete all nodes in linkedlist exept dummy head node
            void clear(){
                std::unique_lock<std::mutex> lock_prev(head->mutex_);
                Node<Value>* prev= head;
                
                std::unique_lock<std::mutex> lock_curr;
                Node<Value>* curr;
                
                while(prev->next!=nullptr){
                    lock_curr=std::unique_lock<std::mutex>(prev->next->mutex_);
                    curr= prev->next;
                    prev->next = curr->next;
                    lock_curr.unlock();
                    delete curr;
                }
            }



            // function to display current list elements, while displaying updates are possible to elements- so this is for testing purpose only
            // when we call display function, we will take lock on cout, so no need to worry about taking lock on cout here
            void display_list(){
                std::unique_lock<std::mutex> lock_prev(head->mutex_);
                Node<Value>* prev= head;
                if(prev->next== nullptr){   // empty list
                    return;
                }

                std::unique_lock<std::mutex> lock_curr(prev->next->mutex_);
                Node<Value>* curr= prev->next;
                
                std::cout<<curr->getValue()<<" ";

                while(curr->next!= nullptr){
                    lock_prev.unlock();
                    prev= curr;
                    lock_prev.swap(lock_curr); // transferig lock ownership from curr to prev
                    
                    lock_curr=std::unique_lock<std::mutex>(prev->next->mutex_);
                    curr= prev->next;

                    std::cout<<curr->getValue()<<" ";
                }
                
                std::cout<<std::endl;
            }
            

        private:
            Node<Value>* head=nullptr; // we keep extra node as head node, so first node is head->next

    };
}


#endif