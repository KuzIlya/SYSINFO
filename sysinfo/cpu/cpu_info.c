#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <stdio.h>
#endif

#include "cpu.h"

#ifdef _WIN32

void PrintCpuInfo() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    const char* processorArchitecture = "";
    switch (sysInfo.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_AMD64:
            processorArchitecture = "x64 (AMD or Intel)";
            break;
        case PROCESSOR_ARCHITECTURE_ARM:
            processorArchitecture = "ARM";
            break;
        case PROCESSOR_ARCHITECTURE_ARM64:
            processorArchitecture = "ARM64";
            break;
        case PROCESSOR_ARCHITECTURE_IA64:
            processorArchitecture = "Intel Itanium-based";
            break;
        case PROCESSOR_ARCHITECTURE_INTEL:
            processorArchitecture = "x86";
            break;
        case PROCESSOR_ARCHITECTURE_UNKNOWN:
            processorArchitecture = "Unknown architecture";
            break;
    }

    printf("Processor info:\n");
    printf("  Processor Architecture: %s\n", processorArchitecture);
    printf("  Number of Processors: %u\n", sysInfo.dwNumberOfProcessors);
    printf("  Processor Type: %u\n", sysInfo.dwProcessorType);

}

#else

void PrintCpuInfo() {
    FILE *fp = fopen("/proc/cpuinfo", "r");
    if (fp == NULL) {
        perror("Failed to open /proc/cpuinfo");
        return;
    }

    char line[256];
    char architecture[256] = "Unknown architecture";
    int num_processors = 0;
    char model_name[256] = "Unknown processor";

    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "processor", 9) == 0) {
            num_processors++;
        } else if (strncmp(line, "model name", 10) == 0) {
            sscanf(line, "model name : %[^\n]", model_name);
        } else if (strncmp(line, "flags", 5) == 0) {
            if (strstr(line, "lm")) {
                strcpy(architecture, "x64 (AMD or Intel)");
            } else {
                strcpy(architecture, "x86");
            }
        }
    }

    fclose(fp);

    printf("Processor info:\n");
    printf("  Processor Architecture: %s\n", architecture);
    printf("  Number of Processors: %d\n", num_processors);
    printf("  Processor Type: %s\n", model_name);
}

#endif