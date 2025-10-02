#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void* threadFunction(void* args){
    int number = *((int*)args);
    int cnt =0;
    for(int i = 0; i<1e8; i++){
        cnt++;
    }
    printf("Thread number %d\n", number);
}

int main(){
    int numOfThreads = 200;
    pthread_t threads[numOfThreads];
    int ids[numOfThreads];

    for(int i = 0; i<numOfThreads; i++) ids[i] = i;
    
    for(int i = 0; i<numOfThreads; i++){
        pthread_create(&threads[i], NULL, threadFunction, &ids[i]);
    }

    for(int i = 0; i<numOfThreads; i++){
        pthread_join(threads[i], NULL);
    }

    printf("All threads finished \n");
    
    return 0;
}