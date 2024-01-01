#include <iostream>
#include <mutex>
class MyClass {
public:
    void lock() { std::cout << "in lock()\n"; mtx.lock(); }
    void unlock() { std::cout << "in unlock()\n"; mtx.unlock(); }

private:
    std::mutex mtx;  // Nested mutex for synchronization
};

int main() {
    MyClass mtx; // std::mutex mtx is used normally
                 //   but for better understanding use MyClass

    // Acquire the lock using a scope guard to ensure automatic unlocking
    {
        std::lock_guard<decltype(mtx)> scoped_lock(mtx); // Lock the mutex upon construction

        // Perform operations within the critical section
        std::cout << "Accessing shared resource..." << std::endl;
    } // Lock is automatically released when lock goes out of scope

    // The mutex is now unlocked and available for other threads
    return 0;
}
