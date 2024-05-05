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
    sleep(10); // Simulate some work done by the thread
    printf("Thread about to stop running: %d\n", arg);
  return NULL;
}

int main() {
  pthread_t thread;

  // Create a new thread
  for (int i = 0; i < 5; i++) {
     int create_result = pthread_create(&thread, NULL, thread_function, (void *)i);
     if (create_result != 0) {
        perror("pthread_create failed");
        return 1;
     }
     pthread_detach(thread);
  }
  printf("Main thread about to exit\n");


  // The main thread exits here
  return 0;
}
