//gcc -g pthread_demo.c -lpthread
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define BUFFER_SIZE 10
#define NUM_PRODUCERS 2
#define NUM_CONSUMERS 2

int buffer[BUFFER_SIZE];
int in = 0;
int out = 0;

/* Synchronization primitives */
pthread_mutex_t mutex;
sem_t sem_empty;    // Counts available empty slots
sem_t sem_full;     // Counts available full slots

/* Enqueue item */
void enqueue(int item) {
    buffer[in] = item;
    in = (in + 1) % BUFFER_SIZE;
}

/* Dequeue item */
int dequeue() {
    int item = buffer[out];
    out = (out + 1) % BUFFER_SIZE;
    return item;
}

void* producer_thread(void* arg) {
    int id = *(int*)arg;
    int item = 0;

    while (1) {
        item++;

        /* Wait for an empty slot */
        sem_wait(&sem_empty);

        /* Lock buffer */
        pthread_mutex_lock(&mutex);

        enqueue(item);
        printf("[Producer %d] Produced %d\n", id, item);

        /* Unlock buffer */
        pthread_mutex_unlock(&mutex);

        /* Signal that a full slot is available */
        sem_post(&sem_full);

        usleep(100000);
    }
    return NULL;
}

void* consumer_thread(void* arg) {
    int id = *(int*)arg;

    while (1) {
        /* Wait for an available item */
        sem_wait(&sem_full);

        /* Lock buffer */
        pthread_mutex_lock(&mutex);

        int item = dequeue();
        printf("    [Consumer %d] Consumed %d\n", id, item);

        /* Unlock buffer */
        pthread_mutex_unlock(&mutex);

        /* Signal an empty slot */
        sem_post(&sem_empty);

        usleep(150000);
    }
    return NULL;
}

int main() {
    pthread_t producers[NUM_PRODUCERS];
    pthread_t consumers[NUM_CONSUMERS];
    int ids[NUM_PRODUCERS > NUM_CONSUMERS ? NUM_PRODUCERS : NUM_CONSUMERS];

    /* Initialize mutex and semaphores */
    pthread_mutex_init(&mutex, NULL);
    sem_init(&sem_empty, 0, BUFFER_SIZE);
    sem_init(&sem_full, 0, 0);

    /* Create threads */
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        ids[i] = i + 1;
        pthread_create(&producers[i], NULL, producer_thread, &ids[i]);
    }

    for (int i = 0; i < NUM_CONSUMERS; i++) {
        ids[i] = i + 1;
        pthread_create(&consumers[i], NULL, consumer_thread, &ids[i]);
    }

    /* Join threads (never reached in this demo) */
    for (int i = 0; i < NUM_PRODUCERS; i++)
        pthread_join(producers[i], NULL);

    for (int i = 0; i < NUM_CONSUMERS; i++)
        pthread_join(consumers[i], NULL);

    return 0;
}

