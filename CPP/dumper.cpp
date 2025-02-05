#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <thread>
#include <filesystem>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <chrono>
#include <atomic>
#include <csignal> // For signal handling

#ifdef _WIN32
#include <windows.h> // For Sleep()
#else
#include <unistd.h> // For sleep()
#endif

namespace fs = std::filesystem;

std::shared_mutex filename_mutex;
std::set<std::string> found_filenames;
std::atomic<bool> running(true);
std::condition_variable cv;
std::mutex cv_mutex;

void search_directory(const fs::path& dir, const std::string& substring) {
while (1) {
    if (!running) return;

    try {
        for (const auto& entry : fs::recursive_directory_iterator(dir)) {
            if (!running) return;

            if (entry.is_regular_file()) {
                fs::path filename = entry.path().filename();
                std::string filename_str = filename.string(); // Convert to string

                if (filename_str.find(substring) != std::string::npos) {
                    std::unique_lock<std::shared_mutex> lock(filename_mutex);
                    found_filenames.insert(entry.path());
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error accessing directory: " << e.what() << std::endl;
    }
        std::unique_lock<std::mutex> lock(cv_mutex);
        cv.wait_for(lock, std::chrono::seconds(1));

}
}

void dumper_thread() {
    while (running) {
        std::unique_lock<std::mutex> lock(cv_mutex);
        cv.wait_for(lock, std::chrono::seconds(5)); // Dump every 5 seconds (adjust as needed)

        if (!running) return;

        {
            {
            std::shared_lock<std::shared_mutex> lock(filename_mutex);
            for (const auto& filename : found_filenames) {
                std::cout << filename << std::endl;
            }}
            {
            std::unique_lock<std::shared_mutex> lock(filename_mutex);
            found_filenames.clear();
            }
        }
    }
}

void handle_signal(int signal) {
    running = false;
    cv.notify_all(); // Wake up the dumper thread
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: file-finder <directory> <substring1> [<substring2> ...]" << std::endl;
        return 1;
    }

    fs::path root_dir(argv[1]);
    std::vector<std::string> substrings;

    for (int i = 2; i < argc; ++i) {
        substrings.push_back(argv[i]);
    }

    if (!fs::exists(root_dir) || !fs::is_directory(root_dir)) {
        std::cerr << "Invalid root directory: " << root_dir << std::endl;
        return 1;
    }

    std::signal(SIGINT, handle_signal); // Handle Ctrl+C
    std::signal(SIGTERM, handle_signal); // Handle termination signal

    std::vector<std::thread> search_threads;
    for (const auto& substring : substrings) {
        search_threads.emplace_back(search_directory, root_dir, substring);
    }

    std::thread dumper(dumper_thread);

    std::string command;
    while (running) {
        std::cin >> command;
        if (command == "dump") {
            {
                {
                std::cout << "-----Dump comand received.\n";
                std::shared_lock<std::shared_mutex> lock(filename_mutex);
                std::cout << "-----Dump comand locked the mutex.\n";
                for (const auto& filename : found_filenames) {
                    std::cout << filename << std::endl;
                }}
                {
                std::unique_lock<std::shared_mutex> lock(filename_mutex);
                found_filenames.clear();
                }
            }
        } else if (command == "exit") {
            running = false;
            cv.notify_all(); // Wake up the dumper thread
            break;
        }
    }

    for (auto& thread : search_threads) {
        thread.join();
    }
    dumper.join();

    found_filenames.clear();

    return 0;
}
