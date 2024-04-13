#include <stdio.h>
#include <iostream>
#include <pthread.h>
#include <unistd.h>
class foo {
public:
foo(int i) :mI(i) {}
~foo() { sleep(30); std::cout << "destroying instance " << mI << std::endl;}
private:
   int mI;
};
foo st1(1);
foo st2(2);

void *thread_function(void *arg) {
  // This function will run as a separate thread
  for (int i = 0; i < 5; i++) {
    sleep(10); // Simulate some work done by the thread
    printf("Thread running: %d\n", i);
  }
  return NULL;
}

int main() {
  pthread_t thread;

  // Create a new thread
  int create_result = pthread_create(&thread, NULL, thread_function, NULL);
  if (create_result != 0) {
    perror("pthread_create failed");
    return 1;
  }

  printf("Main thread about to exit\n");

  pthread_detach(thread);

  // The main thread exits here
  return 0;
}
