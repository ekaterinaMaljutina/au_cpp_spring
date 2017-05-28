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
        void init_thread(size_t begin, size_t end);

        void stop();

        void run_task();

        std::atomic_bool stop_value_;
        std::condition_variable_any condition_variable_;
        std::recursive_mutex mutex_;

        std::queue<type_function_> queue_;
        std::vector<std::thread> threads_;

        size_t limit_queue_;
        size_t size_thread_;
    };

    void thread_pool::init_thread(size_t begin, size_t end) {
        for (size_t i = begin; i < end; ++i) {
            threads_.push_back(std::thread(&thread_pool::run_task, this));
        }
    }

    thread_pool::thread_pool(size_t threads_count, size_t max_queue_size) :
            stop_value_(false), limit_queue_(max_queue_size), size_thread_(threads_count) {
        if (threads_count <= 0 || max_queue_size <= 0) {
            throw std::runtime_error("count < 0");
        }
        init_thread(0, size_thread_);
    }

    void thread_pool::run_task() {
        while (!stop_value_) {
            type_function_ current_task;
            {
                std::unique_lock<std::recursive_mutex> lock(mutex_);
                condition_variable_.wait(lock,
                                         [this] {
                                             return !queue_.empty() || stop_value_;
                                         }
                );
                if (stop_value_ || queue_.empty()) {
                    return;
                }
                current_task = std::move(queue_.front());
                queue_.pop();
                condition_variable_.notify_all();
            }
            current_task();
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
            if (stop_value_)
                return result;
            condition_variable_.wait(lock, [this] {
//                    fprintf(stdout, "queue = %zu  limit = %zu \n", queue_.size(), limit_queue_);
                return queue_.size() < limit_queue_;
            });

            queue_.emplace([current_task]() {
                (*current_task)();
            });

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
        std::unique_lock<std::recursive_mutex> lock(mutex_);
        fprintf(stdout, "current thread size = %zu, set thread count %zu\n", size_thread_, threads_count);
        if (threads_count <= 0)
            throw std::runtime_error("count thread < 0");

        if (size_thread_ < threads_count) {
            init_thread(size_thread_, threads_count);
            size_thread_ = threads_count;
            return;
        }
        stop_value_ = true;
        for (auto &thread:threads_) {
            fprintf(stdout, "wait join in set count \n");
            thread.join();
        }
        threads_.clear();
        stop_value_ = false;
        size_thread_ = threads_count;
        init_thread(0, size_thread_);
    }

    void thread_pool::set_max_queue_size(size_t max_queue_size) {
        if (max_queue_size > 0)
            limit_queue_ = max_queue_size;
        else
            throw std::runtime_error("count queue < 0");
    }

    void thread_pool::stop() {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        stop_value_ = true;
    }

    thread_pool::~thread_pool() {
        stop();
        condition_variable_.notify_all();
        for (auto &thread : threads_) {
            thread.join();
        }
    }
}
// namespace au
