// spinless and lockless semaphore for fast path acquire.
#include <mutex>
#include <queue>
#include <iostream>
#include <thread>
#include <atomic>
#include <condition_variable>

class lockless_spinless_counting_semaphore {
private:
    std::atomic<int> count;
    std::mutex mtx; // Protects the wait queue
    std::condition_variable cv;
    std::queue<std::pair<std::thread::id, bool*>> wait_queue; // Queue of waiting threads and their flags

public:
    explicit lockless_spinless_counting_semaphore(int initial_count = 0) : count(initial_count) {}

    void acquire() {
        int expected = count.load(std::memory_order_relaxed);
        if (expected > 0 && count.compare_exchange_strong(expected, expected - 1, std::memory_order_acquire)) {
            return; // Fast path: acquired without waiting
        }

        std::unique_lock<std::mutex> lock(mtx);
        bool wait_flag = true;
        wait_queue.push({std::this_thread::get_id(), &wait_flag});
        cv.wait(lock, [this, &wait_flag] { return !wait_flag || count.load(std::memory_order_relaxed) > 0; });

        if (!wait_flag) return; // Woke up because it was our turn.

        int expected2 = count.load(std::memory_order_relaxed);
        while (!count.compare_exchange_weak(expected2, expected2 - 1, std::memory_order_acquire, std::memory_order_relaxed));
    }

    void release() {
        {
            std::unique_lock<std::mutex> lock(mtx);
            if (!wait_queue.empty()) {
                auto [thread_id, flag] = wait_queue.front();
                wait_queue.pop();
                *flag = false; // Signal the waiting thread
                cv.notify_one();
                return;
            }
        }
        count.fetch_add(1, std::memory_order_release);
    }
};

int main() {
    lockless_spinless_counting_semaphore sem(2);

    auto worker = [&sem](int id) {
        sem.acquire();
        std::cout << "Thread " << id << " acquired the semaphore" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        std::cout << "Thread " << id << " released the semaphore" << std::endl;
        sem.release();
    };

    std::vector<std::thread> threads;
    for (int i = 1; i <= 4; ++i) {
        threads.emplace_back(worker, i);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    return 0;
}
