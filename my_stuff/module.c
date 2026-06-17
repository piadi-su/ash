#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <ifaddrs.h>
#include <arpa/inet.h>

// Funzione per leggere il primo valore di un file
void read_file_value(const char *path, char *buffer, size_t size) {
    FILE *file = fopen(path, "r");
    if (!file) {
        snprintf(buffer, size, "Unknown");
        return;
    }
    if (!fgets(buffer, size, file)) {
        snprintf(buffer, size, "Unknown");
    } else {
        buffer[strcspn(buffer, "\n")] = 0;
    }
    fclose(file);
}

// OS e Kernel
void print_system_info() {
    struct utsname uts;
    if (uname(&uts) == -1) {
        perror("uname");
        return;
    }
    printf("OS: %s %s\n", uts.sysname, uts.release);
    printf("Kernel: %s\n", uts.version);
}

// Uptime
void print_uptime() {
    struct sysinfo info;
    if (sysinfo(&info) == -1) {
        perror("sysinfo");
        return;
    }
    int days = info.uptime / 86400;
    int hours = (info.uptime % 86400) / 3600;
    int minutes = (info.uptime % 3600) / 60;
    printf("Uptime: %d days, %d hours, %d minutes\n", days, hours, minutes);
}

// IP Locale
void print_local_ip() {
    struct ifaddrs *ifaddr, *ifa;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return;
    }

    printf("Local IP: ");
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) continue;
        if (ifa->ifa_addr->sa_family == AF_INET) {
            void *addr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            inet_ntop(AF_INET, addr, host, NI_MAXHOST);
            if (strcmp(host, "127.0.0.1") != 0) { // ignora localhost
                printf("%s", host);
                break;
            }
        }
    }
    printf("\n");
    freeifaddrs(ifaddr);
}

// CPU
void print_cpu_info() {
    FILE *file = fopen("/proc/cpuinfo", "r");
    char buffer[256];
    if (!file) {
        printf("CPU: Unknown\n");
        return;
    }
    while (fgets(buffer, sizeof(buffer), file)) {
        if (strncmp(buffer, "model name", 10) == 0) {
            char *colon = strchr(buffer, ':');
            if (colon) {
                printf("CPU:%s\n", colon + 2);
                break;
            }
        }
    }
    fclose(file);
}

// RAM e Swap
void print_ram_swap_info() {
    FILE *file = fopen("/proc/meminfo", "r");
    char line[256];
    long total_ram = 0, free_ram = 0, total_swap = 0, free_swap = 0;

    if (!file) {
        printf("RAM/Swap: Unknown\n");
        return;
    }

    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "MemTotal:", 9) == 0) sscanf(line + 9, "%ld", &total_ram);
        else if (strncmp(line, "MemAvailable:", 13) == 0) sscanf(line + 13, "%ld", &free_ram);
        else if (strncmp(line, "SwapTotal:", 10) == 0) sscanf(line + 10, "%ld", &total_swap);
        else if (strncmp(line, "SwapFree:", 9) == 0) sscanf(line + 9, "%ld", &free_swap);
    }
    fclose(file);

    printf("RAM: %ld MB total, %ld MB free\n", total_ram/1024, free_ram/1024);
    printf("Swap: %ld MB total, %ld MB free\n", total_swap/1024, free_swap/1024);
}

// GPU
void print_gpu_info() {
    printf("GPU: ");
    fflush(stdout);
    system("lspci | grep -E 'VGA|3D'");
}

// Monitor (risoluzione) - richiede xrandr
void print_monitor_info() {
    printf("Monitor: ");
    fflush(stdout);
    system("xrandr | grep '*' | awk '{print $1}'");
}

// Shell e Terminale
void print_shell_terminal() {
    char *shell = getenv("SHELL");
    char *term = getenv("TERM");
    printf("Shell: %s\n", shell ? shell : "Unknown");
    printf("Terminal: %s\n", term ? term : "Unknown");
}

int main() {
    printf("=== System Info ===\n\n");

    print_system_info();
    print_uptime();
    print_local_ip();
    print_cpu_info();
    print_ram_swap_info();
    print_gpu_info();
    print_monitor_info();
    print_shell_terminal();

    printf("\n===================\n");
    return 0;
}


