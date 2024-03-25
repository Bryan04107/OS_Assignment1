#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define LOWER_NUM 1
#define UPPER_NUM 10000
#define BUFFER_SIZE 100
#define MAX_COUNT 10000

int buffer[BUFFER_SIZE];
int count = 0;
int total_consumed = 0;
int producer_done = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t producer_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t consumer_cond = PTHREAD_COND_INITIALIZER;

void* producer(void* arg) {
    FILE* all_file = fopen("all.txt", "w");
    if (all_file == NULL) {
        perror("Error opening all.txt");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < MAX_COUNT; i++) {
        pthread_mutex_lock(&mutex);
        while (count == BUFFER_SIZE) {
            pthread_cond_wait(&producer_cond, &mutex);
        }

        int num = LOWER_NUM + rand() % (UPPER_NUM - LOWER_NUM + 1);
        buffer[count++] = num;
        fprintf(all_file, "%d\n", num);
        fflush(all_file);
        fflush(stdout);
        pthread_cond_signal(&consumer_cond);
        pthread_mutex_unlock(&mutex);
    }
    fclose(all_file);
    printf("Producer done\n");
    fflush(stdout);
    producer_done = 1;
    pthread_cond_broadcast(&consumer_cond);
    return NULL;
}

void* consumer_even(void* arg) {
    FILE* even_file = fopen("even.txt", "w");
    if (even_file == NULL) {
        perror("Error opening even.txt");
        exit(EXIT_FAILURE);
    }
    while (1) { 
        pthread_mutex_lock(&mutex);
        while (count == 0 && !producer_done) {
            pthread_cond_wait(&consumer_cond, &mutex);
        }
        if (count == 0 && producer_done) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        if (buffer[count - 1] % 2 == 0) {
            fprintf(even_file, "%d\n", buffer[--count]);
            fflush(even_file);
            fflush(stdout);
            total_consumed++;
            fflush(stdout);
        }
        pthread_cond_signal(&producer_cond);
        pthread_mutex_unlock(&mutex);
    }
    fclose(even_file);
    printf("Even done\n");
    fflush(stdout);
    return NULL;
}

void* consumer_odd(void* arg) {
    FILE* odd_file = fopen("odd.txt", "w");
    if (odd_file == NULL) {
        perror("Error opening odd.txt");
        exit(EXIT_FAILURE);
    }
    while (1) {
        pthread_mutex_lock(&mutex);
        while (count == 0 && !producer_done) {
            pthread_cond_wait(&consumer_cond, &mutex);
        }
        if (count == 0 && producer_done) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        if (buffer[count - 1] % 2 != 0) {
            fprintf(odd_file, "%d\n", buffer[--count]);
            fflush(odd_file);
            fflush(stdout);
            total_consumed++;
            fflush(stdout);
        }
        pthread_cond_signal(&producer_cond);
        pthread_mutex_unlock(&mutex);
    }
    fclose(odd_file);
    printf("Odd done\n");
    fflush(stdout);
    return NULL;
}

int main() {
    pthread_t producer_thread, consumer_even_thread, consumer_odd_thread;
    srand(time(NULL));

    pthread_create(&producer_thread, NULL, producer, NULL);
    pthread_create(&consumer_even_thread, NULL, consumer_even, NULL);
    pthread_create(&consumer_odd_thread, NULL, consumer_odd, NULL);

    pthread_join(producer_thread, NULL);
    pthread_join(consumer_even_thread, NULL); 
    pthread_join(consumer_odd_thread, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&producer_cond);
    pthread_cond_destroy(&consumer_cond);

    return 0;
}

