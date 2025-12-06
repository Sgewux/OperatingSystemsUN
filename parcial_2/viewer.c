#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include "utils.h"

#define MAX_HOSTS 4

int main() {
    int shmid;
    struct host_info *shm_hosts;

    /* ===== Conectarse a memoria compartida ===== */
    shmid = shmget(0x1234, sizeof(struct host_info) * MAX_HOSTS, 0666);
    if (shmid < 0) {
        perror("shmget");
        exit(1);
    }

    shm_hosts = (struct host_info *)shmat(shmid, NULL, 0);
    if (shm_hosts == (void *)-1) {
        perror("shmat");
        exit(1);
    }

    printf("[Viewer] Memoria compartida conectada.\n");

    /* ===== Loop principal ===== */
    while (1) {
        printf("\n====================== VIEWER ======================\n");

        for (int i = 0; i < MAX_HOSTS; i++) {
            if (shm_hosts[i].ip[0] == '\0')
                continue;  // slot vacío

            printf("Host %d (%s)\n", i, shm_hosts[i].ip);
            printf("  CPU Usage:   %.2f%%\n", shm_hosts[i].cpu_usage);
            printf("  CPU User:    %.2f%%\n", shm_hosts[i].cpu_user);
            printf("  CPU System:  %.2f%%\n", shm_hosts[i].cpu_system);
            printf("  CPU Idle:    %.2f%%\n", shm_hosts[i].cpu_idle);
            printf("  Mem Used:    %.2f MB\n", shm_hosts[i].mem_used_mb);
            printf("  Mem Free:    %.2f MB\n", shm_hosts[i].mem_free_mb);
            printf("  Swap Total:  %.2f MB\n", shm_hosts[i].swap_total_mb);
            printf("  Swap Free:   %.2f MB\n", shm_hosts[i].swap_free_mb);
            printf("---------------------------------------------------\n");
        }

        sleep(1);
    }

    return 0;
}
