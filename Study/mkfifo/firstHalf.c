#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define ll long long
#define ld long double

int main(){
    const char* fifo = "/tmp/pi_fifo";

    struct stat st;

    // Create it if it doesn't exist
    if (stat(fifo, &st) == -1) {
        if (mkfifo(fifo, 0666) == -1) {
            perror("mkfifo");
            exit(1);
        }
    }

    ll half = 500e6;
    ld firstTerms = 0;

    for(ll i = 0; i < half; i++){
        if(i & 1){
            firstTerms = firstTerms - (ld)1/(2*i + 1);
        } else {
            firstTerms = firstTerms + (ld)1/(2*i+1);
        }
    }
    
    int fd = open(fifo, O_WRONLY);
    write(fd, &firstTerms, sizeof(firstTerms));
    close(fd);

    return 0;
}