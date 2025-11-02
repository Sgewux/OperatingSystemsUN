#include <stdio.h>      // printf, perror
#include <stdlib.h>     // malloc, free, exit
#include <string.h>     // funciones de cadena (opcional)
#include <unistd.h>     // fork, pipe, read, write, close
#define ll long long
#define ld long double

int main() {
    int fd[2]; 
    char buffer[5];  
    int r;         
    pid_t pid;
    ll half = 500e6;  

    r = pipe(fd); // Creates two file descriptors
    // fd[0] -> Read End
    // fd[1] -> Write End
    // Data Written to fd[1] can be read from fd[0]

    pid = fork();

    if (pid == 0) {   // Child process

        close(fd[0]); // Child doesn't need to read
        ld firstTerms = 0;

        for(ll i = 0; i < half; i++){
            if(i & 1){
                firstTerms = firstTerms - (ld)1/(2*i + 1);
            } else {
                firstTerms = firstTerms + (ld)1/(2*i+1);
            }
        }
        
        write(fd[1], &firstTerms, sizeof(firstTerms));
        close(fd[1]);
        exit(0);

    } else {  
        close(fd[1]); //Parent doesn't write

        ld pi;
        read(fd[0], &pi, sizeof(pi)); // Waits for first half from pipe
        close(fd[0]);

        for(ll i = half; i < 2*half; i++){
            if(i & 1){
                pi = pi - (ld)1/(2*i + 1);
            } else {
                pi = pi + (ld)1/(2*i+1);
            }
        }

        pi = 4*pi;
        printf("%.10Lf\n", pi);
    }

    return 0;
}