#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <sched.h>
#include <unistd.h>

#define MAX_THREADS 8
#define N 1000 // total numbers

long D[N];
long Psum = 0;
int workload;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void init_array() {
    for (int i = 0; i < N; i++){
        D[i] = 1;
    }
}

void bind_to_cpu(int cpu){
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu, &cpuset);

    if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) != 0){
        perror("pthread_setaffinity_np");
        exit(1);
    }
}

void* sum_array(void* arg)
{   
    int tid = *(int*) arg;
    int Si = tid * workload;
    int Ei = Si + workload;
    long sum = 0;
    for (int i = Si; i < Ei; i++){
        sum += D[i];
    }

    pthread_mutex_lock(&lock);
    Psum+= sum;
    pthread_mutex_unlock(&lock);
    return NULL;
}


void* worker(void* arg){

    int tid = *(int*)arg;
    int num_cpus = sysconf(_SC_NPROCESSORS_ONLN);

    printf("Detected CPUs: %d\n", num_cpus);

    int CPU_id = tid % num_cpus;

    bind_to_cpu(CPU_id);
    printf("Thread %d assigned to CPU %d\n", tid, CPU_id);

    sum_array(arg);

    return NULL;

}

int main(){
    pthread_t threads[MAX_THREADS];
    int tids[MAX_THREADS];

    init_array();
    workload = N / MAX_THREADS;

    // Creating threads
    for (int i = 0; i < MAX_THREADS; i++){
        tids[i] = i;
        if (pthread_create(&threads[i], NULL, worker, &tids[i]) != 0)
        {
            perror("pthread_create");
            return 1;
        }
    }

    // Joining threads
    for (int i = 0; i < MAX_THREADS; i++){
        pthread_join(threads[i], NULL);
    }

    printf("Total sum = %ld\n", Psum);

    return 0;
}