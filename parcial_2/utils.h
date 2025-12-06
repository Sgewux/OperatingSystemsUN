#ifndef UTILS_H
#define UTILS_H

#define DEFAULT_PORT 9000 

struct host_info {
    char ip[32];
    float cpu_usage;
    float cpu_user;
    float cpu_system;
    float cpu_idle;
    float mem_used_mb;
    float mem_free_mb;
    float swap_total_mb;
    float swap_free_mb;
};

#endif
