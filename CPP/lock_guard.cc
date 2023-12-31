#include <iostream>
#include <mutex>

int main() {
    std::mutex mtx;

    // Acquire the lock using a scope guard to ensure automatic unlocking
    {
        std::lock_guard<std::mutex> lock(mtx); // Lock the mutex upon construction

        // Perform operations within the critical section
        std::cout << "Accessing shared resource..." << std::endl;
    } // Lock is automatically released when lock goes out of scope

    // The mutex is now unlocked and available for other threads
    return 0;
}
