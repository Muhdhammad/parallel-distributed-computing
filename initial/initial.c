// A single thread that gets bind to different CPU cores, it runs do_work() on CPU 0, then CPU 1, then CPU 2, ..

#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <sched.h>
#include <unistd.h>

#define ITERATIONS 1000000
#define ROUNDS 3

void bind_to_cpu(int cpu){
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu, &cpuset);

    if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) != 0){
        perror("pthread_setaffinity_np");
        exit(1);
    }
}

/*
volatile tells don't optimize access to this var, always read/write directly from memory
Normally, C compilers try to optimize code: They may store variables in CPU registers
But with volatile: Compiler assumes the variable can change at any time outside of normal code flow
So it always fetches the value from memory and writes it back, never skipping or caching it
*/

void do_work()
{
    volatile double x = 1.0001; 
    volatile double y = 1.0002;
    volatile double z = 1.0;

    for (long i = 0; i < ITERATIONS; i++)
    {
        for (long i = 0; i < 100; i++)
        {
            z = z * x * y;
        }
    }
}

void* worker(void* arg){
    int num_cpus = sysconf(_SC_NPROCESSORS_ONLN);

    printf("Detected CPUs: %d\n", num_cpus);

    for (int r = 0; r < ROUNDS; r++){
        printf("\nRound: %d\n", r+1);

        for (int cpu = 0; cpu < num_cpus; cpu++){
            bind_to_cpu(cpu);
            printf("Running on CPU %d\n", cpu);

            do_work();

            // small pause so utilization is visible
            usleep(200000);
        }
    }

    return NULL;
}

int main(){
    pthread_t thread1;

    if (pthread_create(&thread1, NULL, worker, NULL) != 0){
        perror("pthread_create");
        return 1;
    }

    pthread_join(thread1, NULL);

    return 0;
}