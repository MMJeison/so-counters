#include <stdio.h>
#include <string.h>

int main() {
    int numCPUs = 0;
    FILE *cpuinfo = fopen("/proc/cpuinfo", "r");

    if (cpuinfo == NULL) {
        perror("Error al abrir /proc/cpuinfo");
        return 1;
    }

    char line[256];
    while (fgets(line, sizeof(line), cpuinfo)) {
        if (strstr(line, "processor")) {
            numCPUs++;
        }
    }

    fclose(cpuinfo);

    printf("Número de CPUs: %d\n", numCPUs);

    return 0;
}
