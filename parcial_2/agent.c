#include "utils.h"
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

void appendMem(char* buffer, size_t size) {
    FILE *f = fopen("/proc/meminfo", "r");
    if (!f) {
        perror("fopen");
        return;
    }

    char line[256];
    long mem_total_kb = 0, mem_available_kb = 0, mem_free_kb = 0, swap_total_kb = 0, swap_free_kb = 0;

    while (fgets(line, sizeof(line), f)) {
        sscanf(line, "MemTotal: %ld kB", &mem_total_kb);
        sscanf(line, "MemAvailable: %ld kB", &mem_available_kb);
        sscanf(line, "MemFree: %ld kB", &mem_free_kb);
        sscanf(line, "SwapTotal: %ld kB", &swap_total_kb);
        sscanf(line, "SwapFree: %ld kB", &swap_free_kb);
    }

    fclose(f);

    const char* ip_logica = "0.0.0.0"; // Placeholder

    float mem_used_mb  = (mem_total_kb - mem_available_kb) / 1024.0f;
    float mem_free_mb  = mem_free_kb / 1024.0f;
    float swap_total_mb = swap_total_kb / 1024.0f;
    float swap_free_mb  = swap_free_kb / 1024.0f;

    int len = strlen(buffer);
    snprintf(buffer + len, size - len,
             "MEM;%s;%.2f;%.2f;%.2f;%.2f\n",
             ip_logica,
             mem_used_mb,
             mem_free_mb,
             swap_total_mb,
             swap_free_mb);
}


void appendCPU(char* buffer, size_t size) {
    FILE *f = fopen("/proc/stat", "r");
    if (!f) {
        perror("fopen");
        return;
    }

    char cpu_label[5];
    unsigned long user, nice, system, idle, iowait, irq, softirq, steal;

    if (fscanf(f, "%4s %lu %lu %lu %lu %lu %lu %lu %lu",
               cpu_label,
               &user, &nice, &system, &idle,
               &iowait, &irq, &softirq, &steal) != 9) {
        fprintf(stderr, "No se pudo leer la línea de cpu\n");
        fclose(f);
        return;
    }

    fclose(f);

    unsigned long CPU_total = user + nice + system + idle +
                              iowait + irq + softirq + steal;
    unsigned long CPU_idle = idle;

    float CPU_usage = 100.0f * (CPU_total - CPU_idle) / CPU_total;
    float user_pct  = 100.0f * user  / CPU_total;
    float system_pct = 100.0f * system / CPU_total;
    float idle_pct  = 100.0f * idle  / CPU_total;

    const char* ip_logica = "0.0.0.0"; // Placeholder

    int len = strlen(buffer);
    snprintf(buffer + len, size - len,
             "CPU;%s;%.2f;%.2f;%.2f;%.2f\n",
             ip_logica,
             CPU_usage,
             user_pct,
             system_pct,
             idle_pct);
}

int main() {
    char buffer[512];
    const char* server_ip = "127.0.0.1"; // Update your server IP
    int server_port = DEFAULT_PORT;

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);

    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sock);
        return 1;
    }

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sock);
        return 1;
    }

    printf("Connected to server!\n");

    while (1) {
        buffer[0] = '\0';
        appendCPU(buffer, sizeof(buffer));
        appendMem(buffer, sizeof(buffer));

        printf("Sending:\n%s", buffer);

        ssize_t sent = send(sock, buffer, strlen(buffer), 0);
        if (sent < 0) {
            perror("send");
            break; // Exit loop to reconnect or terminate
        }

        sleep(2);
    }

    close(sock);
    return 0;
}
