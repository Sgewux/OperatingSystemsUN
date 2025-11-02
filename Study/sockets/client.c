#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>   // socket, connect, inet_ functions

#define ll long long
#define ld long double

int main() {

    ll half = 500e6;
    ld firstTerms = 0;

    for(ll i = 0; i < half; i++){
        if(i & 1){
            firstTerms = firstTerms - (ld)1/(2*i + 1);
        } else {
            firstTerms = firstTerms + (ld)1/(2*i+1);
        }
    }

    int sock;
    struct sockaddr_in server_addr;
    

    //  Create a socket (IPv4 + TCP)
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket");
        exit(1);
    }

    //  Configure the server address to connect to
    server_addr.sin_family = AF_INET;            // IPv4
    server_addr.sin_port = htons(8080);          // Port 8080, converted to network byte order

    // inet_pton() = “presentation to numeric”
    // Converts a human-readable IP (e.g. "127.0.0.1") into a numeric binary form for the socket
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        exit(1);
    }

    //  Connect to the server
    // If the server is listening on 127.0.0.1:8080, this call will succeed
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        exit(1);
    }

    //  Send a message to the server
    write(sock, &firstTerms, sizeof(firstTerms));

    //  Close the socket when done
    close(sock);
    return 0;
}
