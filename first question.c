#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define BUFFER_SIZE 10

int buffer[BUFFER_SIZE];
int count = 0;
int sum = 0;

pthread_mutex_t mutex;
sem_t full, empty;

void *producer(void *arg) {
    while (1) {
        int num = rand() % 100;

        sem_wait(&empty);
        pthread_mutex_lock(&mutex);

        buffer[count] = num;
        printf("تولید کننده: %d تولید کرد\n", num);
        count++;

        pthread_mutex_unlock(&mutex);
        sem_post(&full);

        sleep(1);
    }
}

void *consumer(void *arg) {
    while (1) {
        sem_wait(&full);
        pthread_mutex_lock(&mutex);

        int num = buffer[count - 1];
        count--;
        sum += num;
        printf("مصرف کننده: %d مصرف کرد، مجموع فعلی: %d\n", num, sum);

        pthread_mutex_unlock(&mutex);
        sem_post(&empty);

        sleep(2);
    }
}

int main() {
    pthread_t prod_thread, cons_thread;

    pthread_mutex_init(&mutex, NULL);
    sem_init(&full, 0, 0);
    sem_init(&empty, 0, BUFFER_SIZE);

    pthread_create(&prod_thread, NULL, producer, NULL);
    pthread_create(&cons_thread, NULL, consumer, NULL);

    pthread_join(prod_thread, NULL);
    pthread_join(cons_thread, NULL);

    pthread_mutex_destroy(&mutex);
    sem_destroy(&full);
    sem_destroy(&empty);

    return 0;
}
