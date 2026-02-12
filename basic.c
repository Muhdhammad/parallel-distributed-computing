#include <stdio.h>
#include <pthread.h>

#define ITERATIONS 1000000000

void* computation(void* num);
void* sum(void* num);

int main(){

    pthread_t thread1;
    pthread_t thread2;

    long num1 = 10;
    long num2 = 20;

    // pthread_create(&thread1, NULL, computation, (void*) &num1);

    /*
    sum((void*) &num1);
    sum((void*) &num2);
    */

    pthread_create(&thread1, NULL, sum, (void*) &num1);
    pthread_create(&thread2, NULL, sum, (void*) &num2);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    
    return 0;

}

void* computation(void* num){
    printf("Hello from threads");
    long *num1 = (long*) (num) ;

    printf("The num is %ld\n", *num1);
    return NULL;
}

void* sum(void* num){
    long sum = 0;
    long* add_num = (long*) (num);
    for (long i = 0; i < ITERATIONS; i++){
        sum += *add_num;
    }
    return NULL;
}