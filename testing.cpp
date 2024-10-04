#include "LRU_cache_system.h"
#include <thread>
#include <mutex>
#include <iostream>
#include <chrono>
#include <barrier>

std::mutex mt; // for locking cout to avoid multiple threads printing at the same time
std::barrier<> br(2);    // for synchronization

void thread1work(LRU::LRUCacheSystem<int,int>& cache){
    std::unique_lock<std::mutex> lk(mt);
    std::cout<<"stage 1"<<"\n";
    lk.unlock();
    

    cache.get_Value(400);
    cache.get_Value(300);
    std::this_thread::sleep_for(std::chrono::microseconds(1));
    cache.get_Value(200);
    cache.get_Value(100);

    lk.lock();
    cache.display_list();
    lk.unlock();

    /*
    the elements if exists in list should be in this order -> 500 600 700 800 900 1000
    and 100 200 300 400

    get_value is working fine here
    */

    
    lk.lock();
    std::cout<<"\n";
    std::cout<<"stage 2"<<"\n";
    lk.unlock();


    br.arrive_and_wait();

    cache.update(1100, 111);   // 1100 is not in cache, it will be brought into cache and its value is updated in cache only not in memory
    int value;
    cache.find_in_map(1100, value);
    lk.lock();
    std::cout<<"value corresponding to 1100 in cache= "<<value<<std::endl; // should give 111
    lk.unlock();    

    cache.memory.find_value(1100, value);
    lk.lock();
    std::cout<<"value corresponding to 1100 in memory= "<<value<<std::endl; // should give 11
    lk.unlock();  


    br.arrive_and_wait();
    // 500 and 600 are updated in thread 2 before this barrier

    cache.find_in_map(500, value);
    lk.lock();
    std::cout<<"value corresponding to 500 in cache= "<<value<<std::endl; // should give 55
    lk.unlock();    

    cache.memory.find_value(500, value);
    lk.lock();
    std::cout<<"value corresponding to 500 in memory= "<<value<<std::endl; // should give 5
    lk.unlock();  




    

}


void thread2work(LRU::LRUCacheSystem<int,int>& cache){

    cache.get_Value(1000);
    cache.get_Value(900);
    cache.get_Value(800);
    cache.get_Value(700);
    cache.get_Value(600);
    cache.get_Value(500);

    
    br.arrive_and_wait();

    /*
    1000 will surely be no present in cache as size of cache is 5, result of display list in thread1 shows that 1000 is not present
    in linked list, so checking whether it is removed from map correctly or not;
    */

    
    int value;
    std::unique_lock<std::mutex> lk(mt);
    std::cout<<"Is 1000 present in map in cache =  "<<cache.find_in_map(1000,value)<<std::endl; // should be 0
    lk.unlock();

    /* updating values of keys already present in cache system
    500 and 600 will always be present till this stage, as thread1 sleeps for some time after inserting 400 and 300. So, 1100, 100, 200, 500, 600 will be present in some order in cache, bacause
    of synchronisation using barrier and sleeping thread1
    */
    cache.update(500, 55);
    cache.update(600, 66);


    br.arrive_and_wait();
    

    // 500 , 600 and 1100 will be recently used elements in cache in some order
    lk.lock();
    cache.display_list();
    lk.unlock();





    std::this_thread::sleep_for(std::chrono::microseconds(10));

    lk.lock();
    std::cout<<"\n";
    std::cout<<"stage 3"<<"\n";
    lk.unlock();

    /*
    checking whether updated values are written back to main memory or not
    */
    cache.get_Value(1200);
    cache.get_Value(1300);
    cache.get_Value(1400);
    cache.get_Value(1500);
    cache.get_Value(1600);

    lk.lock();
    cache.display_list();
    lk.unlock();

    lk.lock();
    std::cout<<"updated values of 500, 600 and 1100 should be written back to memory at this stage"<<"\n";
    lk.unlock();

    cache.memory.find_value(500, value);
    lk.lock();
    std::cout<<"value corresponding to 500 in memory= "<<value<<std::endl; // should give 55
    lk.unlock();  

    cache.memory.find_value(600, value);
    lk.lock();
    std::cout<<"value corresponding to 600 in memory= "<<value<<std::endl; // should give 66
    lk.unlock();  

    cache.memory.find_value(1100, value);
    lk.lock();
    std::cout<<"value corresponding to 1100 in memory= "<<value<<std::endl; // should give 111
    lk.unlock();  

    
}

int main(){

    
    LRU::LRUCacheSystem<int,int> cache(5);

    // putting some elements in main memory 
    cache.memory.insert(100,1);  //(key,value);
    cache.memory.insert(200,2);
    cache.memory.insert(300,3);
    cache.memory.insert(400,4);
    cache.memory.insert(500,5);
    cache.memory.insert(600,6);
    cache.memory.insert(700,7);
    cache.memory.insert(800,8);
    cache.memory.insert(900,9);
    cache.memory.insert(1000,10);
    cache.memory.insert(1100,11);
    cache.memory.insert(1200,12);
    cache.memory.insert(1300,13);
    cache.memory.insert(1400,14);
    cache.memory.insert(1500,15);
    cache.memory.insert(1600,16);
    cache.memory.insert(1700,17);
    cache.memory.insert(1800,18);
    cache.memory.insert(1900,19);
    cache.memory.insert(2000,20);
    // 20 elements in main memory




    std::thread thread1(thread1work,std::ref(cache));
    std::thread thread2(thread2work,std::ref(cache));

    thread1.join();
    thread2.join();
    return 0;
}