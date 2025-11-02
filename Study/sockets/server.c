#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>   // socket, bind, listen, accept, inet_ functions

#define ll long long
#define ld long double

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    //  Create a socket (IPv4 + TCP)
    // AF_INET = IPv4
    // SOCK_STREAM = TCP (reliable, connection-oriented)
    // Protocol 0 = default for given type (TCP here)
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        exit(1);
    }

    //  Fill in the server address structure
    server_addr.sin_family = AF_INET;            // IPv4
    server_addr.sin_port = htons(8080);          // Port 8080 (htons = “host to network short”)
                                                 // Converts from CPU byte order to network order (big-endian)
    server_addr.sin_addr.s_addr = INADDR_ANY;    // Accept connections from any local IP

    //  Bind the socket to the specified IP and port
    // This “reserves” the port so clients can connect
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        exit(1);
    }

    //  Start listening for incoming connections
    // 5 = maximum pending connections queue length
    if (listen(server_fd, 5) == -1) {
        perror("listen");
        exit(1);
    }

    printf("Server waiting for connection...\n");

    //  Accept a connection (blocking call)
    // When a client connects, accept() returns a new socket descriptor for that connection
    client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
    if (client_fd == -1) {
        perror("accept");
        exit(1);
    }

    //  Receive data from the client
    ld pi;
    ll half = 500e6;
    read(client_fd, &pi, sizeof(pi));

    for(ll i = half; i < 2*half; i++){
        if(i & 1){
            pi = pi - (ld)1/(2*i + 1);
        } else {
            pi = pi + (ld)1/(2*i+1);
        }
    }

    pi = 4*pi;
    printf("%.10Lf\n", pi);

    // Close both sockets
    close(client_fd);
    close(server_fd);
    return 0;
}
