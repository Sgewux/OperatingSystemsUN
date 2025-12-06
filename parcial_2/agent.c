/* =========================================================
 * Agente de CPU y Memoria
 * Este archivo contiene DOS implementaciones independientes:
 *   1) agent_cpu.c
 *   2) agent_mem.c
 * Cada una debe compilarse por separado usando defines o
 * separando el archivo manualmente en tu estructura final.
 * ========================================================= */

/***************************************************
 *                  agent_cpu
 ***************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

// Lee la línea de CPU de /proc/stat
// Devuelve: user, nice, system, idle
int run_agent_cpu(const char *collector_ip, int port, const char *logical_ip) {
    int sock;
    struct sockaddr_in server;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("socket"); return 1; }

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    inet_pton(AF_INET, collector_ip, &server.sin_addr);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect"); return 1;
    }

    while (1) {
        FILE *f = fopen("/proc/stat", "r");
        if (!f) { perror("fopen"); exit(1); }

        char lbl[5];
        unsigned long u,n,s,i;
        fscanf(f, "%4s %lu %lu %lu %lu", lbl, &u,&n,&s,&i);
        fclose(f);

        unsigned long total = u+n+s+i;
        float cpu_usage = 100.0f * (total - i) / total;
        float user_pct  = 100.0f * u / total;
        float sys_pct   = 100.0f * s / total;
        float idle_pct  = 100.0f * i / total;

        char buffer[256];
        snprintf(buffer, sizeof(buffer), "CPU;%s;%.2f;%.2f;%.2f;%.2f
",
                 logical_ip, cpu_usage, user_pct, sys_pct, idle_pct);

        send(sock, buffer, strlen(buffer), 0);
        sleep(1);
    }

    close(sock);
    return 0;
}
}


/***************************************************
 *                  agent_mem
 ***************************************************/

// Lee /proc/meminfo
int run_agent_mem(const char *collector_ip, int port, const char *logical_ip) {
    int sock;
    struct sockaddr_in server;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("socket"); return 1; }

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    inet_pton(AF_INET, collector_ip, &server.sin_addr);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect"); return 1;
    }

    while (1) {
        FILE *f = fopen("/proc/meminfo", "r");
        if (!f) { perror("fopen"); exit(1); }

        char line[256];
        long total=0, avail=0, free_m=0, swap_t=0, swap_f=0;

        while (fgets(line, sizeof(line), f)) {
            sscanf(line, "MemTotal: %ld kB", &total);
            sscanf(line, "MemAvailable: %ld kB", &avail);
            sscanf(line, "MemFree: %ld kB", &free_m);
            sscanf(line, "SwapTotal: %ld kB", &swap_t);
            sscanf(line, "SwapFree: %ld kB", &swap_f);
        }
        fclose(f);

        float used_mb = (total - avail) / 1024.0f;
        float free_mb = free_m / 1024.0f;
        float swap_t_mb = swap_t / 1024.0f;
        float swap_f_mb = swap_f / 1024.0f;

        char buffer[256];
        snprintf(buffer, sizeof(buffer), "MEM;%s;%.2f;%.2f;%.2f;%.2f
",
                 logical_ip, used_mb, free_mb, swap_t_mb, swap_f_mb);

        send(sock, buffer, strlen(buffer), 0);
        sleep(1);
    }

    close(sock);
    return 0;
}
}

/***************************************************
 * Nota:
 * En la estructura final separa este archivo en:
 *   agente/agent_cpu.c
 *   agente/agent_mem.c
 * Aquí se incluye todo junto solo por conveniencia del canvas.
 ***************************************************/

