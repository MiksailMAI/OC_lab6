#pragma once

#ifndef SEM_HPP
#define SEM_HPP

#include <mutex>
#include <condition_variable>

class binsem {
public:
    explicit binsem(int init_count = count_max)
      : count(init_count) {}

    void wait()
    {
        std::unique_lock<std::mutex> lk(m_);
        cv_.wait(lk, [=]{ return 0 < count; });
        --count;
    }
    bool try_wait()
    {
        std::lock_guard<std::mutex> lk(m_);
        if (0 < count) {
            --count;
            return true;
        } else {
            return false;
        }
    }

    void signal()
    {
        std::lock_guard<std::mutex> lk(m_);
        if (count < count_max) {
            ++count;
            cv_.notify_one();
        }
    }

    void lock() { wait(); }
    bool try_lock() { return try_wait(); }
    void unlock() { signal(); }

private:
    static const int count_max = 1;
    int count;
    std::mutex m_;
    std::condition_variable cv_;
};


#endif