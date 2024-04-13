#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

void *thread_function(void *arg) {
  // This function will run as a separate thread
  for (int i = 0; i < 5; i++) {
    printf("Thread running: %d\n", i);
    sleep(1); // Simulate some work done by the thread
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
