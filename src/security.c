#define _POSIX_C_SOURCE 200809L
#include "security.h"
#include "process.h"

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int path_has_indicator(const char *path, const char **reason)
{
    static const char *prefixes[] = {
        "/tmp/", "/var/tmp/", "/dev/shm/", "/run/user/"
    };
    size_t i;
    if (strstr(path, " (deleted)") != NULL) {
        *reason = "executable file appears to have been deleted while process is running";
        return 1;
    }
    for (i = 0; i < sizeof(prefixes) / sizeof(prefixes[0]); i++) {
        size_t n = strlen(prefixes[i]);
        if (strncmp(path, prefixes[i], n) == 0) {
            *reason = "executable is located in a commonly writable temporary/runtime directory";
            return 1;
        }
    }
    return 0;
}

void check_suspicious_process(const char *pid)
{
    char link[512], exe[4096];
    ssize_t n;
    const char *reason = NULL;
    if (!is_pid_directory(pid)) {
        puts("Invalid PID.");
        return;
    }
    snprintf(link, sizeof(link), "/proc/%s/exe", pid);
    n = readlink(link, exe, sizeof(exe) - 1);
    puts("\n--- Suspicious executable-path check ---");
    printf("PID: %s\n", pid);
    if (n < 0) {
        puts("Result: Could not inspect executable path (process may have exited or access is restricted).");
        return;
    }
    exe[n] = '\0';
    printf("Executable: %s\n", exe);
    if (path_has_indicator(exe, &reason)) {
        puts("Result: WARNING - suspicious indicator found");
        printf("Reason: %s\n", reason);
        puts("Next step: investigate the file, owner, command line, and context.");
    } else {
        puts("Result: No executable-path indicator found.");
    }
    puts("Important: this heuristic does not confirm or rule out malware.");
}

void scan_processes_for_indicators(void)
{
    DIR *d = opendir("/proc");
    struct dirent *e;
    int inspected = 0, flagged = 0, inaccessible = 0;
    if (!d) { perror("Cannot open /proc"); return; }
    puts("\nScanning accessible process executable paths...");
    while ((e = readdir(d)) != NULL) {
        char link[512], exe[4096];
        ssize_t n;
        const char *reason = NULL;
        if (!is_pid_directory(e->d_name)) continue;
        snprintf(link, sizeof(link), "/proc/%s/exe", e->d_name);
        n = readlink(link, exe, sizeof(exe) - 1);
        if (n < 0) { inaccessible++; continue; }
        exe[n] = '\0';
        inspected++;
        if (path_has_indicator(exe, &reason)) {
            printf("[WARNING] PID %-7s executable=%s\n", e->d_name, exe);
            printf("          Reason: %s\n", reason);
            flagged++;
        }
    }
    closedir(d);
    printf("\nScan summary: inspected=%d, flagged=%d, inaccessible/raced=%d\n",
           inspected, flagged, inaccessible);
    puts("A flag is a lead for investigation, not proof of malware.");
}
