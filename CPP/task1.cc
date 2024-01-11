#include <iostream>
#include <queue>
#include <chrono>
#include <functional>
#include <mutex>
#include <condition_variable>
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
        condition.notify_one();
    }

    void enqueueTask(std::function<void()> task, std::chrono::steady_clock::time_point time) {
        std::lock_guard<std::mutex> lock(queueMutex);
        taskQueue.push({std::move(task), time});
        //condition.notify_one();
    }

    void start() {
        taskThread = std::thread(&DelayedTaskExecutor::processTasks, this);
    }

    void stop() {
        running = false;
        condition.notify_one();
        taskThread.join();
    }

private:
    void processTasks() {
        while (running) {
            TaskEntry task;
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                condition.wait(lock, [this] {
                    return !taskQueue.empty() || !running;
                });
                if (!running) return;
                task = std::move(taskQueue.top());
                taskQueue.pop();
            }

            auto now = std::chrono::steady_clock::now();
            if (task.scheduledTime <= now) {
                std::cout << "from if ";
                task.task();
            } else {
                std::unique_lock<std::mutex> lock(queueMutex);
                condition.wait_until(lock, task.scheduledTime, [this] {
                    return !running || !taskQueue.empty() || taskQueue.top().scheduledTime <= std::chrono::steady_clock::now();
                });

                if (!running) return;

                if (!taskQueue.empty() && taskQueue.top().scheduledTime <= std::chrono::steady_clock::now()) {
                    std::cout << "continueing in else\n";
                    continue;
                }

#if 1
                std::cout << "from else ";
                task.task();
#else
                if (task.scheduledTime <= std::chrono::steady_clock::now()) {
                    std::cout << "ready to run task\n";
                    task.task();
                } else {
                    std::cout << "not ready to run task\n";
                    enqueueTask(task.task, task.scheduledTime); //put it back on.
                }
#endif
            }
        }
    }

    std::priority_queue<TaskEntry, std::vector<TaskEntry>, std::greater<TaskEntry>> taskQueue;
    std::mutex queueMutex;
    std::condition_variable condition;
    std::thread taskThread;
    std::atomic<bool> running = (true);
};

bool operator>(const TaskEntry& a, const TaskEntry& b) {
    return a.scheduledTime > b.scheduledTime;
}

int main() {
    DelayedTaskExecutor executor;
    std::cout << "Starting task processing...\n";
    executor.start();
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
    executor.stop();
    return 0;
}
