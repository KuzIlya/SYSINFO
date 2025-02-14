#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#include <lmcons.h>
#else
#include <unistd.h>
#include <sys/utsname.h>
#endif

#include "host_info.h"

#ifdef _WIN32

void PrintDnsDomainName() {
    TCHAR buffer[256] = TEXT("");
    DWORD dwSize = _countof(buffer);
    
    if (!GetComputerNameEx(ComputerNameDnsHostname, buffer, &dwSize))
    {
        printf(TEXT("GetComputerNameEx failed (%d)\n"), GetLastError());
        return;
    }
    
    printf(TEXT("Hostname: %s\n"), buffer);
}

void PrintUserName() {
    char username[UNLEN + 1];
    DWORD username_len = sizeof(username);

    if (GetUserNameA(username, &username_len)) {
        printf("Current user: %s\n", username);
    } else {
        printf("Failed to get username. Error: %lu\n", GetLastError());
    }
}

#else

void PrintDnsDomainName() {
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        printf("Hostname: %s\n", hostname);
    } else {
        perror("gethostname");
    }
}

void PrintUserName() {
    char *username = getenv("USER");
    if (username) {
        printf("Current user: %s\n", username);
    } else {
        perror("getenv");
    }
}

#endif

void PrintHostInfo() {
    PrintDnsDomainName();
    PrintUserName();
}