#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32

#pragma comment(lib, "pdh.lib")
#include <math.h>
#include <signal.h>
#include <windows.h>
#include <pdh.h>
#include <pdhmsg.h>

#else

#include <sys/statvfs.h>
#include <sys/sysinfo.h>

#endif

#include "plot.h"

#include "gnuplot/gnuplot.h"
#define GP(CMD) write_gp(gnuplot, CMD)

#define DATA_POINTS 60

FILE *gnuplot = NULL;
FILE *data_file = NULL;

#ifdef _WIN32

double GetCpuUsage() {
    FILETIME idleTime, kernelTime, userTime;
    static FILETIME prev_idleTime = {0, 0}, prev_kernelTime = {0, 0}, prev_userTime = {0, 0};

    if (GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
        ULONGLONG idle_diff = (ULONGLONG)idleTime.dwLowDateTime - prev_idleTime.dwLowDateTime + 
                              ((ULONGLONG)(idleTime.dwHighDateTime - prev_idleTime.dwHighDateTime) << 32);
        ULONGLONG kernel_diff = (ULONGLONG)kernelTime.dwLowDateTime - prev_kernelTime.dwLowDateTime + 
                                ((ULONGLONG)(kernelTime.dwHighDateTime - prev_kernelTime.dwHighDateTime) << 32);
        ULONGLONG user_diff = (ULONGLONG)userTime.dwLowDateTime - prev_userTime.dwLowDateTime + 
                              ((ULONGLONG)(userTime.dwHighDateTime - prev_userTime.dwHighDateTime) << 32);

        ULONGLONG total_diff = kernel_diff + user_diff;
        ULONGLONG idle_diff_total = idle_diff;

        double cpu_usage = (1.0 - (double)idle_diff_total / total_diff) * 100.0;

        prev_idleTime = idleTime;
        prev_kernelTime = kernelTime;
        prev_userTime = userTime;

        return cpu_usage;
    }
}

double GetMemoryUsage() {
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    if (GlobalMemoryStatusEx(&statex)) {
        return (double)(statex.dwMemoryLoad);
    }
    return 0.0;
}

double GetDiskUsage() {
    PDH_HQUERY hQuery;
    PDH_HCOUNTER hCounterDiskTime;
    PDH_FMT_COUNTERVALUE counterValueDiskTime;
    
    PdhOpenQuery(NULL, 0, &hQuery);
    
    PdhAddCounter(hQuery, "\\PhysicalDisk(_Total)\\Disk Time", 0, &hCounterDiskTime);

    PdhCollectQueryData(hQuery);
    
    Sleep(1000);
    
    PdhCollectQueryData(hQuery);
    
    PdhGetFormattedCounterValue(hCounterDiskTime, PDH_FMT_DOUBLE, NULL, &counterValueDiskTime);
    
    PdhCloseQuery(hQuery);

    return counterValueDiskTime.doubleValue;
}

#else

static unsigned long long last_total, last_idle;
double GetCpuUsage() {
    FILE *file = fopen("/proc/stat", "r");
    if (!file) return -1;
    
    unsigned long long user, nice, system, idle;
    fscanf(file, "cpu  %llu %llu %llu %llu", &user, &nice, &system, &idle);
    fclose(file);
    
    unsigned long long total = user + nice + system + idle;
    if (last_total > 0) {
        unsigned long long total_diff = total - last_total;
        unsigned long long idle_diff = idle - last_idle;
        last_total = total;
        last_idle = idle;
        return (100.0 * (total_diff - idle_diff)) / total_diff;
    }
    
    last_total = total;
    last_idle = idle;
    return 0.0;
}

double GetMemoryUsage() {
    struct sysinfo memInfo;
    if (sysinfo(&memInfo) != 0) return -1;
    
    long long total_memory = memInfo.totalram;
    total_memory *= memInfo.mem_unit;
    long long free_memory = memInfo.freeram;
    free_memory *= memInfo.mem_unit;
    
    double used_memory = 100.0 * (1.0 - (double)free_memory / total_memory);
    return used_memory;
}

static unsigned long long last_disk_time = 0;
static struct timespec last_time = {0, 0};

double GetDiskUsage() {
    FILE *file = fopen("/proc/diskstats", "r");
    if (!file) return -1.0;
    
    char line[256];
    unsigned long long total_time = 0;
    
    while (fgets(line, sizeof(line), file)) {
        unsigned int major, minor;
        char device[32];
        unsigned long long rd_ios, rd_merges, rd_sectors, rd_ticks;
        unsigned long long wr_ios, wr_merges, wr_sectors, wr_ticks;
        unsigned long long ios_pgr, total_ticks, rq_ticks;
        
        if (sscanf(line, "%u %u %s %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu", 
                   &major, &minor, device, &rd_ios, &rd_merges, &rd_sectors, &rd_ticks,
                   &wr_ios, &wr_merges, &wr_sectors, &wr_ticks, &ios_pgr, &total_ticks, &rq_ticks) == 14) {
            total_time += total_ticks;
        }
    }
    
    fclose(file);
    
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    if (last_time.tv_sec != 0) {  // Пропускаем первый вызов
        double elapsed_time = (now.tv_sec - last_time.tv_sec) + (now.tv_nsec - last_time.tv_nsec) / 1e9;
        unsigned long long disk_diff = total_time - last_disk_time;
        
        last_disk_time = total_time;
        last_time = now;

        return (disk_diff / (elapsed_time * 10.0)) * 100.0;  // `total_ticks` в миллисекундах → переводим в секунды
    }

    last_disk_time = total_time;
    last_time = now;
    return 0.0;  // Первый вызов всегда 0, т.к. нет разницы
}

#endif

int PlotUsageGraphics(int plot_time) {
    gnuplot = init_gp();

    #ifdef _WIN32
    GP("set terminal windows size 1600,1000");
    #else
    GP("set terminal qt size 1600,1000");
    #endif
    GP("unset mouse");
    GP("set xdata time");
    GP("set timefmt \"%s\"");
    GP("set format x \"%H:%M:%S\"");
    GP("set xlabel 'Time'");
    GP("set ylabel 'Usage (%)'");
    GP("set yrange [-1:100]");
    GP("set key outside");

    data_file = fopen("usage_data.txt", "w");
    if (!data_file) {
        fprintf(stderr, "Error: cannot open file.\n");
        return 1;
    }

    time_t start_time = time(NULL);
    double cpu_usage, memory_usage, disk_usage;

    time_t range_start = start_time;
    time_t range_end = start_time + 60;

    for (int i = 0; i < plot_time; i++) {
        time_t current_time = time(NULL);
        cpu_usage = GetCpuUsage();
        memory_usage = GetMemoryUsage();
        disk_usage = GetDiskUsage();

        fprintf(data_file, "%ld %lf %lf %lf\n", current_time, cpu_usage, memory_usage, disk_usage);
        fflush(data_file);

        if (current_time >= range_end) {
            range_start++;
            range_end++;
        }

        char range_str[50];
        snprintf(range_str, sizeof(range_str), "set xrange [%ld:%ld]", range_start, range_end);

        GP(range_str);

        GP("plot 'usage_data.txt' using 1:2 with lines title 'CPU', "
           "'usage_data.txt' using 1:3 with lines title 'RAM', "
           "'usage_data.txt' using 1:4 with lines title 'DISK'");

        sleep(1);
    }

    fclose(data_file);
    GP("exit");
    close_gp(gnuplot);

    return 0;
}
