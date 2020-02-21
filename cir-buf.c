/*
gcc -g rw.c  -lpthread
*/
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sched.h>

#define x 8
#define N (1<<x)
volatile unsigned long long R=0, W=0;
volatile int shutdown=0;
long long int CB[N] = {-1};
static void handler(int signum)
{
     shutdown=1;
     printf("Requesting Exit\n");
     return;
}
inline int full()
{
    return (R+N==W);
}
inline int empty()
{
   return (R==W);
}
void * writer(void *ignore)
{
   ignore;
   while (!shutdown) {
       while (R+N!=W) { //!full()
              CB[W++&(N-1)] = 1;
        }
        sched_yield();
     }
     printf("Writer exiting. Processed %llu.\n", W);
     pthread_exit(NULL);
}

void * reader(void *ignore)
{
   ignore;
   while (!shutdown) {
       while (R!=W) { //!empty()
              CB[R++&(N-1)] = 0;
        }
        sched_yield();
     }
     printf("Reader exiting. Processed %llu.\n", R);
     pthread_exit(NULL);
}
int main(void)
{
   int err=0;
   struct sigaction sa;
   printf("Starting. Press ^C to stop.\n");
   memset(&sa, 0, sizeof(sa));
   sa.sa_handler=handler;
   err=sigaction(SIGINT, &sa, NULL);
   if (err) {
      err = errno;
      printf("failed to add signal handler for SIGINT, error %d", err);
      return err;
   }
   pthread_t rt, wt;
   err = pthread_create(&rt, NULL, reader, NULL);
   if (err) {
      printf("failed to launch reader, error %d", err);
      return err;
   }
   err = pthread_create(&wt, NULL, writer, NULL);
   if (err) {
      printf("failed to launch writer, error %d", err);
      return err;
   }
   pthread_join(wt, NULL);
   pthread_join(rt, NULL);
   printf("Done %llu %llu %llu\n", R, W, W-R);
   return 0;
}

