#include <stdio.h>
#include <string.h>

void readMem(){
    
}

int main() {
    FILE *f = fopen("/proc/meminfo", "r");
    if (!f) {
        perror("fopen");
        return 1;
    }
    char line[256];
    long mem_total_kb = 0, mem_free_kb = 0;

    while (fgets(line, sizeof(line), f)) {
        if (sscanf(line, "MemTotal: %ld kB", &mem_total_kb) == 1)
            continue;
        if (sscanf(line, "MemFree: %ld kB", &mem_free_kb) == 1)
            continue;
    }

    fclose(f);
    printf("MemTotal = %ld kB (%.2f MB)\n",
    mem_total_kb, mem_total_kb / 1024.0);
    printf("MemFree = %ld kB (%.2f MB)\n",
    mem_free_kb, mem_free_kb / 1024.0);
    
    return 0;
}