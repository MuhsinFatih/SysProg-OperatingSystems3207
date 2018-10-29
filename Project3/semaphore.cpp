#include <mutex>
#include <condition_variable>
#include <stdlib.h>
#include <sys/wait.h>
#include <iostream>

class semaphore {
private:
    std::mutex mutex_;
    std::condition_variable condition_;
    u_long count_ = 0;

public:
    // Constructor
    semaphore(int count = 0){
        count_ = count;
    }


    void notify() { // Vw
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        ++count_;
        condition_.notify_one();
    }

    void wait() { // P
        std::unique_lock<decltype(mutex_)> lock(mutex_);
        while(count_ > 0)
            condition_.wait(lock);
        --count_;
    }

    bool try_wait() {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        if(count_){ 
            --count_;
            return true;
        }
        return false;
    }
};