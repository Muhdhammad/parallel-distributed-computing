#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>

#define MAX_THREADS 8
#define M 256
#define K 256
#define N 256 
// MxK KxN = MxN
float A[M][K];
float B[K][N];
float C[M][N];
int workload;

struct ThreadArg {
    int tid;
    int row_start;
    int row_end;
};

void init_matrices(){
    for (int i=0; i<M; i++){
        for (int j=0; j<K; j++){
            A[i][j] = 1.0f;
        }
    }

    for (int i=0; i<K; i++){
        for (int j=0; j<N; j++){
            B[i][j] = 2.0f;
        }
    }
}

void bind_to_cpu(int cpu){
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu, &cpuset);
    if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) != 0){
        perror("some");
        exit(1);
    }
}



void* worker(void* arg){
    struct ThreadArg* targ = (struct ThreadArg*) arg;
    int tid = targ->tid;
    int row_start = targ->row_start;
    int row_end = targ->row_end;
    int num_cpus = sysconf(_SC_NPROCESSORS_ONLN);

    int cpu_id = tid % num_cpus;

    bind_to_cpu(cpu_id);
    printf("Thread %d -> rows [%d, %d)\n", tid, row_start, row_end);

    for (int i=row_start; i<row_end; i++){
        for (int j=0; j<N; j++){
            float sum = 0.0f;
            for (int k=0; k<K; k++){
                sum+= A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }   
    }

    return NULL;

}

int main() {
    pthread_t threads[MAX_THREADS];
    struct ThreadArg args[MAX_THREADS];

    init_matrices();
    workload = M / MAX_THREADS;

    for (int i=0; i<MAX_THREADS; i++){
        args[i].tid = i;
        args[i].row_start = i * workload; 
        args[i].row_end = args[i].row_start + workload;

        if (pthread_create(&threads[i], NULL, worker, &args[i]) != 0){
            perror("pthread_create");
            exit(1);
        }
    }

    for (int i=0; i<MAX_THREADS; i++){
        pthread_join(threads[i], NULL);
    }

    printf("C[0][0]   = %.1f\n", C[0][0]);
    printf("C[255][255] = %.1f\n", C[255][255]);

    return 0;
}