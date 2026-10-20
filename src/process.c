#define _POSIX_C_SOURCE 200809L
#include "process.h"

#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

int is_pid_directory(const char *name)
{
    size_t i;
    if (name == NULL || name[0] == '\0') return 0;
    for (i = 0; name[i] != '\0'; i++) {
        if (!isdigit((unsigned char)name[i])) return 0;
    }
    return 1;
}

static void print_status_field(const char *pid, const char *wanted, const char *label)
{
    char path[256], line[512];
    FILE *f;
    snprintf(path, sizeof(path), "/proc/%s/status", pid);
    f = fopen(path, "r");
    if (!f) {
        printf("%s: unavailable (process may have exited or access is restricted)\n", label);
        return;
    }
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, wanted, strlen(wanted)) == 0) {
            printf("%s", line);
            fclose(f);
            return;
        }
    }
    fclose(f);
    printf("%s: unavailable\n", label);
}

static void print_command(const char *pid)
{
    char path[256], buf[4096];
    size_t n, i;
    FILE *f;
    snprintf(path, sizeof(path), "/proc/%s/cmdline", pid);
    f = fopen(path, "r");
    if (!f) {
        puts("Command: unavailable");
        return;
    }
    n = fread(buf, 1, sizeof(buf) - 1, f);
    fclose(f);
    if (n == 0) {
        puts("Command: unavailable");
        return;
    }
    buf[n] = '\0';
    for (i = 0; i < n; i++) if (buf[i] == '\0') buf[i] = ' ';
    printf("Command: %s\n", buf);
}

static unsigned long long read_start_ticks(const char *pid)
{
    char path[256], line[2048], *end, *token;
    FILE *f;
    int field;
    unsigned long long result = 0;
    snprintf(path, sizeof(path), "/proc/%s/stat", pid);
    f = fopen(path, "r");
    if (!f) return 0;
    if (!fgets(line, sizeof(line), f)) { fclose(f); return 0; }
    fclose(f);
    end = strrchr(line, ')');
    if (!end) return 0;
    token = strtok(end + 2, " ");
    for (field = 3; field < 22; field++) {
        if (!token) return 0;
        token = strtok(NULL, " ");
    }
    if (token) result = strtoull(token, NULL, 10);
    return result;
}

static void print_start_time(unsigned long long start_ticks)
{
    FILE *f;
    char line[256];
    double uptime;
    long ticks = sysconf(_SC_CLK_TCK);
    time_t now, started;
    struct tm tm_value;
    if (ticks <= 0 || start_ticks == 0) {
        puts("Start time: unavailable");
        return;
    }
    f = fopen("/proc/uptime", "r");
    if (!f) { puts("Start time: unavailable"); return; }
    if (!fgets(line, sizeof(line), f)) { fclose(f); puts("Start time: unavailable"); return; }
    fclose(f);
    if (sscanf(line, "%lf", &uptime) != 1) { puts("Start time: unavailable"); return; }
    now = time(NULL);
    started = now - (time_t)(uptime - ((double)start_ticks / ticks));
    if (!localtime_r(&started, &tm_value)) { puts("Start time: unavailable"); return; }
    printf("Start time: %04d-%02d-%02d %02d:%02d:%02d\n",
           tm_value.tm_year + 1900, tm_value.tm_mon + 1, tm_value.tm_mday,
           tm_value.tm_hour, tm_value.tm_min, tm_value.tm_sec);
}

unsigned long read_process_cpu(const char *pid)
{
    char path[256], line[2048], *end, *token;
    FILE *f;
    unsigned long utime, stime;
    int field;
    snprintf(path, sizeof(path), "/proc/%s/stat", pid);
    f = fopen(path, "r");
    if (!f) return 0;
    if (!fgets(line, sizeof(line), f)) { fclose(f); return 0; }
    fclose(f);
    end = strrchr(line, ')');
    if (!end) return 0;
    token = strtok(end + 2, " ");
    for (field = 3; field < 14; field++) {
        if (!token) return 0;
        token = strtok(NULL, " ");
    }
    if (!token) return 0;
    utime = strtoul(token, NULL, 10);
    token = strtok(NULL, " ");
    if (!token) return 0;
    stime = strtoul(token, NULL, 10);
    return utime + stime;
}

unsigned long long read_system_cpu(void)
{
    FILE *f = fopen("/proc/stat", "r");
    char line[1024];
    unsigned long long user = 0, nice = 0, system = 0, idle = 0;
    unsigned long long iowait = 0, irq = 0, softirq = 0, steal = 0;
    if (!f) return 0;
    if (!fgets(line, sizeof(line), f)) { fclose(f); return 0; }
    fclose(f);
    if (sscanf(line, "cpu %llu %llu %llu %llu %llu %llu %llu %llu",
               &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal) < 4)
        return 0;
    return user + nice + system + idle + iowait + irq + softirq + steal;
}

static int read_io(const char *pid, unsigned long *read_bytes, unsigned long *write_bytes)
{
    char path[256], line[256];
    FILE *f;
    *read_bytes = *write_bytes = 0;
    snprintf(path, sizeof(path), "/proc/%s/io", pid);
    f = fopen(path, "r");
    if (!f) return 0;
    while (fgets(line, sizeof(line), f)) {
        (void)sscanf(line, "read_bytes: %lu", read_bytes);
        (void)sscanf(line, "write_bytes: %lu", write_bytes);
    }
    fclose(f);
    return 1;
}

void analyze_process(const char *pid)
{
    char path[256], exe[4096];
    ssize_t n;
    unsigned long cpu1, cpu2, rb1, wb1, rb2, wb2;
    unsigned long long sys1, sys2;
    long hz = sysconf(_SC_CLK_TCK);
    double cpu_pct = 0.0;
    snprintf(path, sizeof(path), "/proc/%s", pid);
    if (!is_pid_directory(pid) || access(path, F_OK) != 0) {
        printf("PID %s does not appear to exist or is not accessible.\n", pid);
        return;
    }
    printf("\n=== Process information: PID %s ===\n", pid);
    print_status_field(pid, "Name:", "Name");
    print_command(pid);
    print_status_field(pid, "State:", "State");
    print_status_field(pid, "PPid:", "PPid");
    print_status_field(pid, "Uid:", "Uid");
    print_status_field(pid, "VmRSS:", "VmRSS");
    print_status_field(pid, "Threads:", "Threads");
    print_start_time(read_start_ticks(pid));

    snprintf(path, sizeof(path), "/proc/%s/exe", pid);
    n = readlink(path, exe, sizeof(exe) - 1);
    if (n >= 0) {
        exe[n] = '\0';
        printf("Executable: %s\n", exe);
    } else {
        puts("Executable: unavailable (permissions or process state may prevent reading it)");
    }

    cpu1 = read_process_cpu(pid);
    sys1 = read_system_cpu();
    if (!read_io(pid, &rb1, &wb1)) rb1 = wb1 = 0;
    puts("\nMeasuring CPU/I/O activity for 1 second...");
    sleep(1);
    cpu2 = read_process_cpu(pid);
    sys2 = read_system_cpu();
    if (!read_io(pid, &rb2, &wb2)) rb2 = wb2 = 0;
    printf("Process CPU delta: %lu ticks\n", cpu2 >= cpu1 ? cpu2 - cpu1 : 0);
    if (hz > 0) cpu_pct = (double)(cpu2 >= cpu1 ? cpu2 - cpu1 : 0) / (double)hz * 100.0;
    printf("Approx. CPU use: %.2f%% of one CPU core during sample\n", cpu_pct);
    if (rb2 >= rb1 && wb2 >= wb1) {
        printf("I/O read delta: %lu bytes\n", rb2 - rb1);
        printf("I/O write delta: %lu bytes\n", wb2 - wb1);
    } else {
        puts("I/O delta unavailable");
    }
    (void)sys1; (void)sys2;
    puts("Note: process data can change or disappear while it is being read.");
}

void list_processes(void)
{
    DIR *d = opendir("/proc");
    struct dirent *e;
    int count = 0;
    if (!d) { perror("Cannot open /proc"); return; }
    puts("\nPID       PROCESS NAME");
    puts("------------------------------");
    while ((e = readdir(d)) != NULL) {
        char path[512], name[256] = "unknown";
        FILE *f;
        if (!is_pid_directory(e->d_name)) continue;
        snprintf(path, sizeof(path), "/proc/%s/comm", e->d_name);
        f = fopen(path, "r");
        if (f) {
            if (!fgets(name, sizeof(name), f)) strcpy(name, "unknown");
            name[strcspn(name, "\n")] = '\0';
            fclose(f);
        }
        printf("%-9s %s\n", e->d_name, name);
        count++;
    }
    closedir(d);
    printf("Total processes listed: %d\n", count);
}
