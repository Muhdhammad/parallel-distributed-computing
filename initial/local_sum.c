#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>

#define MAX_THREADS 8
#define N 1000

long nums[N];
long partial_sums[MAX_THREADS];
int workload;

void init_array(){
    for (int i = 0; i < N; i++){
        nums[i] = 1;
    }
}

void bind_to_cpu(int cpu){
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu, &cpuset);

    if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) != 0){
        perror("pthread_setaffinity_np");
        exit(1);
    };
}

void* sum_array(void* arg){
    int tid = *(int*) arg;
    int Si = tid * workload;
    int Ei = Si + workload;
    long sum = 0;

    for (int i=Si; i<Ei; i++){
        sum += nums[i];
    }

    partial_sums[tid] = sum;
    return NULL;
}

void* worker(void* arg){
    int num_cpus = sysconf(_SC_NPROCESSORS_ONLN);
    int tid = *(int*) arg;

    printf("Detected num of cpus %d\n", num_cpus);
    int cpu_id = tid % num_cpus;
    bind_to_cpu(cpu_id);
    printf("Thread %d bind to cpu %d\n", tid, cpu_id);

    sum_array(arg);
    return NULL;
}

int main(){
    pthread_t threads[MAX_THREADS];
    int tids[MAX_THREADS];

    init_array();
    workload = N / MAX_THREADS;

    for (int i=0; i<MAX_THREADS; i++){
        tids[i] = i;
        if (pthread_create(&threads[i], NULL, worker, &tids[i]) != 0) {
            perror("Error while creating threads");
            exit(1);
        }
    }

    for (int i=0; i<MAX_THREADS; i++){
        pthread_join(threads[i], NULL);
    }

    long total_sum = 0;
    for (int i=0; i<MAX_THREADS; i++){
        total_sum += partial_sums[i];
    }

    printf("Total sum: %ld\n", total_sum);

    return 0;
}