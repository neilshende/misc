// condition_variable::wait_for example
#include <iostream>           // std::cout
#include <thread>             // std::thread
#include <chrono>             // std::chrono::seconds
#include <mutex>              // std::mutex, std::unique_lock
#include <condition_variable> // std::condition_variable, std::cv_status

std::condition_variable cv;
std::mutex mtx;
int value;

void read_value() {
  for (;;) {
     std::cin >> value;
     std::lock_guard<decltype(mtx)> lck(mtx);
     cv.notify_one();
     if (value==911) break;
  }
}

int main ()
{
  std::cout << "Please, enter an integer (I'll be printing dots): \n";
  std::thread th (read_value);

  for (;;) {
     std::unique_lock<decltype(mtx)> lck(mtx);
     while (cv.wait_for(lck,std::chrono::seconds(1))==std::cv_status::timeout) {
        std::cout << '.' << std::flush;
     }
     std::cout << "You entered: " << value << '\n';
     if (value==911) break;
  }

  th.join();

  return 0;
}
