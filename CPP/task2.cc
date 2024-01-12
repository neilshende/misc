#include <iostream>
#include <queue>
#include <chrono>
#include <functional>
#include <mutex>
#include <thread>

struct TaskEntry {
   std::function<void()> task;
   std::chrono::steady_clock::time_point scheduledTime;
};


class DelayedTaskExecutor {
public:
    void enqueueTask(std::function<void()> task, std::chrono::milliseconds delay) {
        std::lock_guard<std::mutex> lock(queueMutex);
        taskQueue.push({std::move(task), std::chrono::steady_clock::now() + delay});
    }

    DelayedTaskExecutor( DelayedTaskExecutor &) = delete;

    void operator=(const DelayedTaskExecutor &) = delete;

    static DelayedTaskExecutor & getInstance() {
        static DelayedTaskExecutor instance;
        return instance;
    }
private:
    DelayedTaskExecutor() : running(true), taskThread(std::thread(&DelayedTaskExecutor::processTasks, this)) {
//        running = true;
//        taskThread = std::thread(&DelayedTaskExecutor::processTasks, this);
    }

    ~DelayedTaskExecutor() {
        running = false;
        taskThread.join();
        std::cout << "Done\n";
    }

    void processTasks() {
        while (running) {
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                if (!taskQueue.empty() && taskQueue.top().scheduledTime <= std::chrono::steady_clock::now()) {
                    auto task = std::move(taskQueue.top());
                    taskQueue.pop();
                    lock.unlock();
                    task.task();
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    std::priority_queue<TaskEntry, std::vector<TaskEntry>, std::greater<TaskEntry>> taskQueue;
    std::mutex queueMutex;
    std::thread taskThread;
    std::atomic<bool> running;
};


bool operator>(const TaskEntry& a, const TaskEntry& b) {
    return a.scheduledTime > b.scheduledTime;
}

int main() {
    std::cout << "Starting task processing...\n";
    DelayedTaskExecutor &executor = DelayedTaskExecutor::getInstance();
    //executor.start();
    std::cout << "**Main thread continues execution and can enqueue tasks anytime**\n";

    // Test cases:
    executor.enqueueTask([] { std::cout << "Task 1 (immediate)\n"; }, std::chrono::milliseconds(0));
    executor.enqueueTask([] { std::cout << "Task 2 (delay 2s)\n"; }, std::chrono::seconds(2));
    executor.enqueueTask([] { std::cout << "Task 3 (delay 1s)\n"; }, std::chrono::seconds(1));
    executor.enqueueTask([] { std::cout << "Task 4 (delay 0.5s)\n"; }, std::chrono::milliseconds(500));
    std::this_thread::sleep_for(std::chrono::seconds(1));
    executor.enqueueTask([] { std::cout << "Task 5 (delay 1/2 s)\n"; }, std::chrono::milliseconds(5000));
    std::this_thread::sleep_for(std::chrono::seconds(1));
    executor.enqueueTask([] { std::cout << "Task 6 (delay 5000s)\n"; }, std::chrono::seconds(5000));

    std::this_thread::sleep_for(std::chrono::seconds(10));
    executor.enqueueTask([] { std::cout << "Task 7 (delay 7s)\n"; }, std::chrono::seconds(7));
    std::this_thread::sleep_for(std::chrono::seconds(10));
    std::cout << "Called Stop\n";

    return 0;
}
