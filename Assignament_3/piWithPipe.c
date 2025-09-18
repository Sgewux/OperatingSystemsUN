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

    r = pipe(fd);
    pid = fork();

    if (pid == 0) {  
        close(fd[0]);
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
        close(fd[1]);
        ld pi;
        read(fd[0], &pi, sizeof(pi));
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