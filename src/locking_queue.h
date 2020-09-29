#pragma once

#include <condition_variable>
#include <deque>
#include <mutex>

//! Simple locking queue
template <typename T> class LockingQueue
{
    std::deque<T> q;
    std::mutex m;
    std::condition_variable cv;
    using guard = std::unique_lock<decltype(m)>;

public:
    LockingQueue(LockingQueue&& other) noexcept
    {
        guard lock(other.m);
        q = std::move(other.q);
    }

    LockingQueue() = default;

    void enqueue(T const& item)
    {
        {
            guard lock(m);
            q.push_back(item);
        }
        cv.notify_one();
    }

    void enqueue(T&& item)
    {
        {
            guard lock(m);
            q.push_back(std::forward<T>(item));
        }
        cv.notify_one();
    }

    void wait_dequeue(T& item)
    {
        guard lock(m);
        cv.wait(lock, [&] { return !q.empty(); });
        item = q.front();
        q.pop_front();
    }

    bool try_dequeue(T& item)
    {
        guard lock(m);
        if (q.empty()) {
            return false;
        }
        item = q.front();
        q.pop_front();
        return true;
    }

    bool empty()
    {
        guard lock(m);
        return q.empty();
    }
    size_t size()
    {
        guard lock(m);
        return q.size();
    }
};

