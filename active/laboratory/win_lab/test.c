#include <stdio.h>
#include <pthread.h>

void * work_thread(void * data)
{
    printf("Hello from thread.\n");
}

int main(void)
{
    size_t NUM_THREADS = 10;
    pthread_t threads[NUM_THREADS];
    printf("HELLO WINDOWS!\n");
    for(int i=0; i<NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, work_thread, NULL);
    }
}
