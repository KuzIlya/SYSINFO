#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <errno.h>

#include "cpu/cpu.h"
#include "devices/devices.h"
#include "plot/plot.h"
#include "host/host_info.h"

void print_help() {
    printf("Usage: program [options]\n");
    printf("  -h, --help       Show this help message\n");
    printf("  -C, --cpu        Print CPU information\n");
    printf("  -D, --devices    Print devices information\n");
    printf("  -H, --host       Print host information\n");
    printf("  -P, --plot <value>  Plot RAM, CPU, DISK usage with an optional integer value\n");
}

int main(int argc, char *argv[]) {

    bool print_cpu = false;
    bool print_devices = false;
    bool print_host = false;
    bool plot_usage = false;
    int plot_value = 0;  // Default value for plot flag

    if (argc == 1) {
        print_help();
        return 0;
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_help();
            return 0;
        } else if (strcmp(argv[i], "-C") == 0 || strcmp(argv[i], "--cpu") == 0) {
            print_cpu = true;
        } else if (strcmp(argv[i], "-D") == 0 || strcmp(argv[i], "--devices") == 0) {
            print_devices = true;
        } else if (strcmp(argv[i], "-H") == 0 || strcmp(argv[i], "--host") == 0) {
            print_host = true;
        } else if (strcmp(argv[i], "-P") == 0 || strcmp(argv[i], "--plot") == 0) {
            plot_usage = true;
            if (i + 1 < argc) {
                char *endptr;
                plot_value = strtol(argv[i + 1], &endptr, 10);

                if (*endptr != '\0' || errno == ERANGE) {
                    printf("Error: Invalid integer value for -P or --plot: %s\n", argv[i + 1]);
                    return 1;
                }
                i++;
            } else {
                printf("Error: Missing value after -P or --plot\n");
                return 1;
            }
        } else {
            printf("Unrecognized option: %s\n", argv[i]);
            print_help();
            return 1;
        }
    }

    if (print_host) {
        PrintHostInfo();
    }

    if (print_cpu) {
        PrintCpuInfo();
    }

    if (print_devices) {
        PrintDevicesInfo();
    }

    if (plot_usage) {
        printf("Plotting usage with value: %d\n", plot_value);
        PlotUsageGraphics(plot_value);
    }

    return 0;
}
