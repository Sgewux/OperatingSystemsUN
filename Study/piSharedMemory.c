#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     // fork(), getpid(), pipe(), etc.
#include <sys/types.h>  // pid_t
#include <sys/ipc.h>    // ftok(), IPC_PRIVATE, etc.
#include <sys/shm.h>    // shmget(), shmat(), etc.
#include <sys/wait.h>   // wait()

#define ll long long
#define ld long double

int main(){
    int shmid;
    ld *sharedPi;
    ll half = 500e6;
    key_t key = 1234;

    // Create shared memory

    shmid = shmget(key, sizeof(ld), IPC_CREAT); // Creates shared memory segment
    sharedPi = (ld*) shmat(shmid, NULL, 0); // Attaches memory segment to program space
    *sharedPi = 0;

    pid_t pid = fork();

    if(pid == 0){ // Child

        ld firstTerms = 0;

        for(ll i = 0; i < half; i++){
            if(i & 1){
                firstTerms = firstTerms - (ld)1/(2*i + 1);
            } else {
                firstTerms = firstTerms + (ld)1/(2*i+1);
            }
        }

        *sharedPi = firstTerms; // Write first terms directly
        shmdt(sharedPi); // Detaches memory segment
        exit(0);

    } else { // Parent
        
        wait(NULL); // Waits for child
        ld pi = *sharedPi;

        for(ll i = half; i < 2*half; i++){
            if(i & 1){
                pi = pi - (ld)1/(2*i + 1);
            } else {
                pi = pi + (ld)1/(2*i+1);
            }
        }

        pi = 4*pi;
        printf("%.10Lf\n", pi);

        //Clean
        shmdt(sharedPi);
        shmctl(shmid, IPC_RMID, NULL);

    }

    return 0;
}