#include <iostream>
#include <queue>
#include <chrono>
#include <functional>
#include <mutex>
#include <thread>
#include <atomic>
#include <condition_variable>

struct TaskEntry {
   std::function<void()> task;
   std::chrono::steady_clock::time_point scheduledTime;
};


class DelayedTaskExecutor {
public:
    void enqueueTask(std::function<void()> task, std::chrono::milliseconds delay) {
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            taskQueue.push({std::move(task), std::chrono::steady_clock::now() + delay});
        }
        condition.notify_all();
    }

    DelayedTaskExecutor( DelayedTaskExecutor &) = delete;

    void operator=(const DelayedTaskExecutor &) = delete;

    static DelayedTaskExecutor & getInstance() {
        static DelayedTaskExecutor instance;
        return instance;
    }
private:
    DelayedTaskExecutor() : running(true), taskThread(std::thread(&DelayedTaskExecutor::processTasks, this)) {
    }

    ~DelayedTaskExecutor() {
        running = false;
        condition.notify_all();
        taskThread.join();
        std::cout << "queue size " << taskQueue.size() << " looped " << loopCount << std::endl;
    }

    void processTasks() {
        loopCount = 0;
        while (running) {
            {
                ++loopCount;
                std::unique_lock<std::mutex> lock(queueMutex);
                condition.wait_until(lock, std::chrono::steady_clock::now() + std::chrono::milliseconds(500),
                    [this] {
                       return (!running ||
                             (!taskQueue.empty()
                              && taskQueue.top().scheduledTime <= std::chrono::steady_clock::now()
                          ));
                    });
                if (!running) return;
                if (!taskQueue.empty() && taskQueue.top().scheduledTime <= std::chrono::steady_clock::now())
                {
                    auto task = std::move(taskQueue.top());
                    taskQueue.pop();
                    lock.unlock();
                    task.task();
                }
            }
        }
    }

    std::priority_queue<TaskEntry, std::vector<TaskEntry>, std::greater<TaskEntry>> taskQueue;
    std::mutex queueMutex;
    std::thread taskThread;
    std::atomic<bool> running;
    std::condition_variable condition;
    long loopCount;
};


bool operator>(const TaskEntry& a, const TaskEntry& b) {
    return a.scheduledTime > b.scheduledTime;
}

int main() {
    std::cout << "Starting task processing...\n";
    DelayedTaskExecutor &executor = DelayedTaskExecutor::getInstance();
    std::cout << "**Main thread continues execution and can enqueue tasks anytime**\n";

    // Test cases:
    executor.enqueueTask([] { std::cout << "Task 1 (immediate)\n"; }, std::chrono::milliseconds(0));
    executor.enqueueTask([] { std::cout << "Task 2 (delay 2s)\n"; }, std::chrono::seconds(2));
    executor.enqueueTask([] { std::cout << "Task 3 (delay 1s)\n"; }, std::chrono::seconds(1));
    executor.enqueueTask([] { std::cout << "Task 4 (delay 0.5s)\n"; }, std::chrono::milliseconds(500));
    std::this_thread::sleep_for(std::chrono::seconds(1));
    executor.enqueueTask([] { std::cout << "Task 5 (delay 5s)\n"; }, std::chrono::seconds(5));
    std::this_thread::sleep_for(std::chrono::seconds(1));
    executor.enqueueTask([] { std::cout << "Task 6 (delay 5000s)\n"; }, std::chrono::seconds(5000));

    std::this_thread::sleep_for(std::chrono::seconds(10));
    executor.enqueueTask([] { std::cout << "Task 7 (delay 7s)\n"; }, std::chrono::milliseconds(7000));
    std::this_thread::sleep_for(std::chrono::seconds(10));
    executor.enqueueTask([] { std::cout << "Task 8 (delay 8s)\n"; }, std::chrono::milliseconds(8000));
    std::this_thread::sleep_for(std::chrono::seconds(14));
    executor.enqueueTask([] { std::cout << "Task 9 (delay 8s)\n"; }, std::chrono::milliseconds(8000));
    std::this_thread::sleep_for(std::chrono::seconds(16));
    std::cout << "Called Stop\n";

    return 0;
}
