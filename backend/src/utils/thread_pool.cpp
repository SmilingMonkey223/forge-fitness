#include "thread_pool.hpp"

namespace forge {

ThreadPool::ThreadPool(size_t num_threads) : stop_(false) {
    for (size_t i = 0; i < num_threads; ++i) {
        workers_.emplace_back([this] {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(this->queue_mutex_);
                    this->condition_.wait(lock, [this] {
                        return this->stop_ || !this->tasks_.empty();
                    });

                    if (this->stop_ && this->tasks_.empty()) {
                        return;
                    }

                    task = std::move(this->tasks_.front());
                    this->tasks_.pop();
                }

                task();
                pending_count_--;
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        stop_ = true;
    }
    condition_.notify_all();

    for (std::thread& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

size_t ThreadPool::pending_tasks() const {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    return tasks_.size();
}

void ThreadPool::wait_all() {
    while (pending_count_ > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

} // namespace forge
