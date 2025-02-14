#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#include <mntent.h>
#endif

#include "devices.h"

#ifdef _WIN32

void PrintMemoryInfo() {
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    
    if (GlobalMemoryStatusEx(&memInfo)) {
        printf("RAM:\n");
        printf("  Total: %llu MB\n", memInfo.ullTotalPhys / (1024 * 1024));
        printf("  Free: %llu MB\n", memInfo.ullAvailPhys / (1024 * 1024));
        printf("  Used: %.2f%%\n", (100.0 - memInfo.dwMemoryLoad));
    } else {
        printf("Error during getting RAM info.\n");
    }
}

void PrintDiskInfo() {
    char drives[256];
    DWORD driveCount = GetLogicalDriveStringsA(sizeof(drives), drives);

    if (driveCount > 0) {
        printf("\nHDD Info:\n");
        char* drive = drives;
        while (*drive) {
            ULARGE_INTEGER freeBytesAvailable, totalBytes, totalFreeBytes;
            
            if (GetDiskFreeSpaceExA(drive, &freeBytesAvailable, &totalBytes, &totalFreeBytes)) {
                printf("  Disk %s\n", drive);
                printf("    Total: %llu GB\n", totalBytes.QuadPart / (1024 * 1024 * 1024));
                printf("    Free: %llu GB\n", totalFreeBytes.QuadPart / (1024 * 1024 * 1024));
            } else {
                printf("Error during printing info for %s\n", drive);
            }
            drive += strlen(drive) + 1;
        }
    } else {
        printf("Error during getting disk list.\n");
    }
}

#else

void PrintMemoryInfo() {
    struct sysinfo memInfo;
    if (sysinfo(&memInfo) == 0) {
        printf("RAM:\n");
        printf("  Total: %lu MB\n", memInfo.totalram / (1024 * 1024));
        printf("  Free: %lu MB\n", memInfo.freeram / (1024 * 1024));
        printf("  Used: %.2f%%\n", 
               100.0 * (1.0 - (double)memInfo.freeram / (double)memInfo.totalram));
    } else {
        printf("Error during getting RAM info.\n");
    }
}

void PrintDiskInfo() {
    FILE *mounts = setmntent("/proc/mounts", "r");
    if (!mounts) {
        printf("Error while open /proc/mounts\n");
        return;
    }
    struct mntent *mnt;
    printf("\nHDD Info:\n");
    while ((mnt = getmntent(mounts)) != NULL) {
        struct statvfs stat;
        if (statvfs(mnt->mnt_dir, &stat) == 0) {
            unsigned long long total = (unsigned long long)stat.f_blocks * stat.f_frsize;
            unsigned long long free = (unsigned long long)stat.f_bfree * stat.f_frsize;
            printf("  Disk %s\n", mnt->mnt_fsname);
            printf("    Total: %llu GB\n", total / (1024 * 1024 * 1024));
            printf("    Free: %llu GB\n", free / (1024 * 1024 * 1024));
        }
    }
    endmntent(mounts);
}

#endif

void PrintDevicesInfo() {
    PrintMemoryInfo();
    PrintDiskInfo();
}