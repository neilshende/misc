#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define BUFFER_SIZE 10
#define NUM_PRODUCERS 2
#define NUM_CONSUMERS 2

int buffer[BUFFER_SIZE];
int in = 0;
int out = 0;
int count = 0;  // number of items in the buffer

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_not_full  = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_not_empty = PTHREAD_COND_INITIALIZER;

void enqueue(int item) {
    buffer[in] = item;
    in = (in + 1) % BUFFER_SIZE;
    count++;
}

int dequeue() {
    int item = buffer[out];
    out = (out + 1) % BUFFER_SIZE;
    count--;
    return item;
}

void* producer_thread(void* arg) {
    int id = *(int*)arg;
    int item = 0;

    while (1) {
        usleep(100000);  // Simulate work
        item++;

        pthread_mutex_lock(&mutex);

        /* Wait if buffer is full */
        while (count == BUFFER_SIZE)
            pthread_cond_wait(&cond_not_full, &mutex);

        enqueue(item);
        printf("[Producer %d] Produced %d\n", id, item);

        /* Signal consumers that buffer is not empty */
        pthread_cond_signal(&cond_not_empty);

        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

void* consumer_thread(void* arg) {
    int id = *(int*)arg;

    while (1) {
        pthread_mutex_lock(&mutex);

        /* Wait if buffer is empty */
        while (count == 0)
            pthread_cond_wait(&cond_not_empty, &mutex);

        int item = dequeue();
        printf("    [Consumer %d] Consumed %d\n", id, item);

        /* Signal producers that buffer has space */
        pthread_cond_signal(&cond_not_full);

        pthread_mutex_unlock(&mutex);

        usleep(150000);  // Simulate processing
    }
    return NULL;
}

int main() {
    pthread_t producers[NUM_PRODUCERS];
    pthread_t consumers[NUM_CONSUMERS];
    int ids[NUM_PRODUCERS > NUM_CONSUMERS ? NUM_PRODUCERS : NUM_CONSUMERS];

    for (int i = 0; i < NUM_PRODUCERS; i++) {
        ids[i] = i + 1;
        pthread_create(&producers[i], NULL, producer_thread, &ids[i]);
    }

    for (int i = 0; i < NUM_CONSUMERS; i++) {
        ids[i] = i + 1;
        pthread_create(&consumers[i], NULL, consumer_thread, &ids[i]);
    }

    /* Join (never reached in demo) */
    for (int i = 0; i < NUM_PRODUCERS; i++)
        pthread_join(producers[i], NULL);

    for (int i = 0; i < NUM_CONSUMERS; i++)
        pthread_join(consumers[i], NULL);

    return 0;
}

