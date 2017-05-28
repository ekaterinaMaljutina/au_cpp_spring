#pragma once

#include <pthread.h>
#include <cstddef>
#include <future>
#include <queue>
#include <iostream>
#include <algorithm>


namespace au {

    class thread_pool {
    private:
        typedef std::function<void()> type_function_;
    public:
        thread_pool() = delete;

        thread_pool(thread_pool const &) = delete;

        thread_pool(thread_pool &&) = delete;

        thread_pool &operator=(thread_pool const &)      = delete;

        thread_pool operator=(thread_pool &&)            = delete;

        explicit thread_pool(size_t threads_count, size_t max_queue_size);

        ~thread_pool();

        template<class function, class... argument>
        auto submit(function func, argument... args)
        -> std::future<typename std::result_of<function(argument...)>::type>;

        void set_threads_count(size_t threads_count);

        void set_max_queue_size(size_t max_queue_size);

        size_t threads_count() const;

        size_t max_queue_size() const;

    private:
        void stop();

        std::atomic_bool stop_value_;
        std::condition_variable_any condition_variable_;
        std::recursive_mutex mutex_;

        std::queue<type_function_> queue_;
        std::vector<std::thread> threads_;

        size_t limit_queue_;
        size_t size_thread_;
    };

    thread_pool::thread_pool(size_t threads_count, size_t max_queue_size) :
            stop_value_(false), limit_queue_(max_queue_size), size_thread_(threads_count) {
        if (threads_count <= 0 || max_queue_size <= 0) {
            throw std::runtime_error("count < 0");
        }

        for (size_t i = 0; i < size_thread_; i++) {
            threads_.push_back(std::thread(
                    [this] {
                        while (!stop_value_ || !queue_.empty()) {
                            fprintf(stdout, "run while \n");
                            type_function_ current_task;
                            {
                                std::unique_lock<std::recursive_mutex> lock(mutex_);
                                fprintf(stdout, "wait 1\n");
                                condition_variable_.wait(lock,
                                                         [this] {
                                                             return stop_value_ || !queue_.empty();
                                                         }
                                );

                                if (queue_.empty() || threads_.size() > size_thread_) {
                                    return;
                                }
                                current_task = std::move(queue_.front());
                                fprintf(stdout, "pop task \n");

                                queue_.pop();
                                condition_variable_.notify_all();
                                lock.unlock();
                            }
                            current_task();
                        }
                    }
            ));
        }
    }

    template<class function, class... argument>
    auto thread_pool::submit(function func, argument... args)
    -> std::future<typename std::result_of<function(argument...)>::type> {

        typedef typename std::result_of<function(argument...)>::type returt_type_;
        typedef std::packaged_task<returt_type_()> task;

        if (stop_value_)
            throw std::runtime_error("thread pool stoped");

        auto current_task = std::make_shared<task>(
                std::bind(std::forward<function>(func), std::forward<argument>(args)...));

        std::future<returt_type_> result = current_task->get_future();
        {
            std::unique_lock<std::recursive_mutex> lock(mutex_);
            if (queue_.size() == limit_queue_) {
                fprintf(stdout, "wait 2 \n");
                condition_variable_.wait(lock, [this] {
                    fprintf(stdout, "queue = %zu  limit = %zu \n", queue_.size(), limit_queue_);
                    return queue_.size() < limit_queue_;
                });

            }
            fprintf(stdout, "push task \n");

            queue_.emplace([current_task]() {
                (*current_task)();
            });
            lock.unlock();
        }
        condition_variable_.notify_one();
        return result;
    };

    size_t thread_pool::threads_count() const {
        return size_thread_;
    }

    size_t thread_pool::max_queue_size() const {
        return limit_queue_;
    }

    void thread_pool::set_threads_count(size_t threads_count) {
        fprintf(stdout, "set thread count = %zu \n", threads_count);
        if (threads_count > 0) {
            size_thread_ = threads_count;
            fprintf(stdout, "current set thread count = %zu \n", size_thread_);
        } else {
            throw std::runtime_error("count thread < 0");
        }
    }

    void thread_pool::set_max_queue_size(size_t max_queue_size) {
        if (max_queue_size > 0)
            limit_queue_ = max_queue_size;
        else
            throw std::runtime_error("count queue < 0");
    }

    void thread_pool::stop() {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        std::cout << "stop thread pool" << std::endl;
        stop_value_ = true;
    }

    thread_pool::~thread_pool() {
        stop();
        condition_variable_.notify_all();
        for (auto iter = threads_.begin(); iter != threads_.end(); iter++) {
            (*iter).join();
        }
    }
}
// namespace au
