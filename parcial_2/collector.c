/* =========================================================
 * Collector (UN SOLO SERVIDOR) + find_or_create_host
 * Arquitectura final:
 *   - Un solo servidor TCP
 *   - Cada máquina tiene UN agente que envía CPU y MEM
 *   - Cada conexión = una máquina = un hilo
 *   - El hilo atiende ambas métricas (CPU+MEM) en la MISMA conexión
 *   - find_or_create_host() asigna un índice por IP lógica
 * ========================================================= */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "utils.h"
#include <sys/ipc.h>
#include <sys/shm.h>

int shmid;
struct host_info *shm_hosts;

#define MAX_HOSTS 4

struct host_info hosts[MAX_HOSTS];
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/* =========================================================
 * Busca o crea una entrada para la IP lógica
 * ========================================================= */
int find_or_create_host(const char *ip) {
    for (int i = 0; i < MAX_HOSTS; i++) {
        if (strcmp(hosts[i].ip, ip) == 0)
            return i;
    }
    for (int i = 0; i < MAX_HOSTS; i++) {
        if (hosts[i].ip[0] == '\0') {
            strncpy(hosts[i].ip, ip, 31);
          //  hosts[i].counter = 0;
            return i;
        }
    }
    return -1; // lleno
}

/* =========================================================
 * Hilo para atender UNA máquina (una conexión)
 * ========================================================= */
void *handle_machine(void *arg) {
    int client_sock = *(int *)arg;
    free(arg);

    char buffer[512];

    while (1) {
        int n = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
        if (n <= 0) break;
        buffer[n] = '\0';

        pthread_mutex_lock(&lock);
        
        char *line = strtok(buffer, "\n");
        while (line) {
            char ip[32];
            float a, b, c, d;
        
            /* ---- CPU ---- */
            if (strncmp(line, "CPU;", 4) == 0) {
                if (sscanf(line, "CPU;%31[^;];%f;%f;%f;%f", ip, &a, &b, &c, &d) == 5) {
                    int idx = find_or_create_host(ip);
                    if (idx >= 0) {
                        hosts[idx].cpu_usage  = a;
                        hosts[idx].cpu_user   = b;
                        hosts[idx].cpu_system = c;
                        hosts[idx].cpu_idle   = d;
                    }
                }
            }
        
            /* ---- MEM ---- */
            else if (strncmp(line, "MEM;", 4) == 0) {
                if (sscanf(line, "MEM;%31[^;];%f;%f;%f;%f", ip, &a, &b, &c, &d) == 5) {
                    int idx = find_or_create_host(ip);
                    if (idx >= 0) {
                        hosts[idx].mem_used_mb   = a;
                        hosts[idx].mem_free_mb   = b;
                        hosts[idx].swap_total_mb = c;
                        hosts[idx].swap_free_mb  = d;
                        
                       // hosts[idx].counter++;
                    }
                }
            }
        
            line = strtok(NULL, "\n");
        }

        /* Copiar tabla completa hacia el segmento compartido */
        memcpy(shm_hosts, hosts, sizeof(struct host_info) * MAX_HOSTS);
        
        pthread_mutex_unlock(&lock);

        printf("[RECV] %s", buffer);
    }

    close(client_sock);
    return NULL;
}

/* =========================================================
 * MAIN - Un solo servidor
 * ========================================================= */
int main() {
    int server_fd;
    struct sockaddr_in server, client;
    socklen_t c = sizeof(client);

    memset(hosts, 0, sizeof(hosts));

    // ===== Crear memoria compartida =====
    shmid = shmget(0x1234, sizeof(struct host_info) * MAX_HOSTS, IPC_CREAT | 0666);
    if (shmid < 0) { perror("shmget"); exit(1); }

    shm_hosts = (struct host_info *)shmat(shmid, NULL, 0);
    if (shm_hosts == (void *)-1) { perror("shmat"); exit(1); }

    memset(shm_hosts, 0, sizeof(struct host_info) * MAX_HOSTS);

    printf("[Collector] Memoria compartida creada y mapeada.\n");

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); return 1; }

    server.sin_family = AF_INET;
    server.sin_port = htons(DEFAULT_PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("bind"); return 1;
    }

    listen(server_fd, 10);
    printf("[Collector] Escuchando en puerto %d...\n", DEFAULT_PORT);

    while (1) {
        int client_sock = accept(server_fd, (struct sockaddr *)&client, &c);
        if (client_sock < 0) { perror("accept"); continue; }

        printf("[Collector] Nueva conexion aceptada.\n");

        int *arg = malloc(sizeof(int));
        *arg = client_sock;

        pthread_t t;
        pthread_create(&t, NULL, handle_machine, arg);
        pthread_detach(t);
    }

    close(server_fd);
    return 0;
}
