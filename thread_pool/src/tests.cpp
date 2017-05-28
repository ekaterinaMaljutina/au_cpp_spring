#include "thread_pool.hpp"
#include "catch.hpp"

using au::thread_pool;


// тривиальный тест
TEST_CASE("one thread task submitting") {
//    thread_pool tp(1, 2);
//    bool runned = false;
//    std::cout << "runned = " << runned << std::endl;
//    tp.submit([&runned]() { runned = true; }).get();
//    std::cout << "runned = " << runned << std::endl;
//    CHECK(runned);
}


// тривиальный тест
TEST_CASE("set thread count") {
//    thread_pool tp(1, 2);
//    CHECK(tp.threads_count() == 1);
//    tp.set_threads_count(10);
//    CHECK(tp.threads_count() == 10);
}


// тривиальный тест
TEST_CASE("set max queue size") {
//    thread_pool tp(1, 2);
//    CHECK(tp.max_queue_size() == 2);
//    tp.set_max_queue_size(10);
//    CHECK(tp.max_queue_size() == 10);
}


// нетривиальный тест
TEST_CASE("recursive submit, set_thread_count") {
    fprintf(stdout, "recursive mutex \n");
    thread_pool tp(1, 1);
    std::function<size_t(size_t)> fib;
    fib = [&tp, &fib](size_t n) -> size_t {
        if (n <= 1) {
            return 1;
        }
        fprintf(stdout, "tp thread size = %zu \n", tp.threads_count());
        tp.set_threads_count(tp.threads_count() + 2);
        tp.set_max_queue_size(tp.max_queue_size() + 2);
        fprintf(stdout, "tp thread size = %zu \n ", tp.threads_count());
        fprintf(stdout, "submit  1 ");
        auto res1 = tp.submit(fib, n - 1);
        fprintf(stdout, "submit  2 \n ");
        auto res2 = tp.submit(fib, n - 2);
        auto a = res1.get();
        auto b = res2.get();
        fprintf(stdout, "res2 = %zu \n", b);
        return a + b;
//        return res1.get() + res2.get();
    };
//    tp.submit(fib, 1).get();
//    CHECK(tp.submit(fib, 1).get() == 1);
    CHECK(tp.submit(fib, 6).get() == 13);
    CHECK(tp.threads_count() == 14);
}


// нетривиальный тест
TEST_CASE("test blocking queue") {
//    size_t thread_count = 0;
//    size_t max_queue_size = 0;
//
//    SECTION("case 1") {
//        max_queue_size = 1;
//        thread_count = 1;
//    }
//
//    SECTION("case 2") {
//        max_queue_size = 13;
//        thread_count = 1;
//    }
//
//    SECTION("case 3") {
//        max_queue_size = 1;
//        thread_count = 13;
//    }
//
//    SECTION("case 4") {
//        max_queue_size = 132;
//        thread_count = 13;
//    }
//
//    thread_pool tp(thread_count, max_queue_size);
//
//    std::mutex mutex;
//    mutex.lock();
//    for (size_t idx = 0; idx < max_queue_size + thread_count; ++idx) {
//        tp.submit([&mutex] { std::unique_lock<std::mutex> lock(mutex); });
//    }
//
//    std::atomic_bool wait_over(false);
//    std::future<void> wait_future;
//    std::thread wait_submit([&tp, &wait_over, &wait_future] {
//        wait_future = tp.submit([&wait_over] { wait_over = true; });
//    });
//
//    std::this_thread::sleep_for(std::chrono::milliseconds(100));
//    CHECK(wait_over == false);
//    mutex.unlock();
//    wait_submit.join();
//    wait_future.get();
//    CHECK(wait_over == true);
}

// Примеры нетривиальных тестов:
// - количество одновременно работающих задач == thread_cout
// - во время set_thread_count возможно использование тредпула (задачи выполняются)
// - во время set_thread_count можно вызвать другой set_thread_count и ничего не ломается
// - сложная конфигурация вызова задач, set_thread_count, set_max_queue работает как ожидается
// - и т.д.
