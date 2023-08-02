#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int count;
    int waiting;
} NoStarveMutex;

void no_starve_mutex_init(NoStarveMutex* nsm) {
    pthread_mutex_init(&nsm->mutex, NULL);
    pthread_cond_init(&nsm->cond, NULL);
    nsm->count = 0;
    nsm->waiting = 0;
}

void no_starve_mutex_lock(NoStarveMutex* nsm) {
    pthread_mutex_lock(&nsm->mutex);
    nsm->waiting++;
    while (nsm->count > 0) {
        pthread_cond_wait(&nsm->cond, &nsm->mutex);
    }
    nsm->waiting--;
    nsm->count = 1;
    pthread_mutex_unlock(&nsm->mutex);
}

void no_starve_mutex_unlock(NoStarveMutex* nsm) {
    pthread_mutex_lock(&nsm->mutex);
    nsm->count = 0;
    if (nsm->waiting > 0) {
        pthread_cond_signal(&nsm->cond);
    }
    pthread_mutex_unlock(&nsm->mutex);
}

void* thread_function(void* arg) {
    NoStarveMutex* nsm = (NoStarveMutex*)arg;
    printf("Thread %ld attempting to acquire mutex.\n", pthread_self());
    no_starve_mutex_lock(nsm);
    printf("Thread %ld acquired mutex.\n", pthread_self());

    // Simulando uma seção crítica.
    sleep(2);

    printf("Thread %ld releasing mutex.\n", pthread_self());
    no_starve_mutex_unlock(nsm);
    printf("Thread %ld released mutex.\n", pthread_self());
    return NULL;
}

int main() {
    NoStarveMutex nsm;
    no_starve_mutex_init(&nsm);

    pthread_t thread1, thread2, thread3;

    pthread_create(&thread1, NULL, thread_function, &nsm);
    pthread_create(&thread2, NULL, thread_function, &nsm);
    pthread_create(&thread3, NULL, thread_function, &nsm);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);

    return 0;
}
