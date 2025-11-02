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
    ld pi = 0.0;

    int fd = open(fifo, O_RDONLY);
    if (fd == -1) {
        perror("Error opening FIFO for reading");
        exit(1);
    }

    ssize_t bytesRead = read(fd, &pi, sizeof(pi));
    close(fd);

    if (bytesRead != sizeof(pi)) {
        fprintf(stderr, "Error: expected %zu bytes but got %zd\n",
                sizeof(pi), bytesRead);
        //unlink(fifo);
        exit(1);
    }

    for(ll i = half; i < 2*half; i++){
        if(i & 1){
            pi = pi - (ld)1/(2*i + 1);
        } else {
            pi = pi + (ld)1/(2*i+1);
        }
    }

    pi = 4*pi;
    printf("%.10Lf\n", pi);
    return 0;
}