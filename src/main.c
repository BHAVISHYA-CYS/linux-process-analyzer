#include <stdio.h>
#include <dirent.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

/* ---------- Function declarations ---------- */

int is_pid_directory(const char *name);

void read_process_name(const char *pid);

void read_process_info(const char *pid);

void read_process_memory(const char *pid);

unsigned long read_process_cpu(const char *pid);

int read_process_io(const char *pid,
                    unsigned long *read_bytes,
                    unsigned long *write_bytes);

unsigned long long read_system_cpu(void);


/* ---------- Check whether directory name is a PID ---------- */

int is_pid_directory(const char *name)
{
    int i;

    if (name == NULL || name[0] == '\0')
    {
        return 0;
    }

    for (i = 0; name[i] != '\0'; i++)
    {
        if (!isdigit((unsigned char)name[i]))
        {
            return 0;
        }
    }

    return 1;
}


/* ---------- Read process name ---------- */

void read_process_name(const char *pid)
{
    char path[256];
    char line[256];

    FILE *file;

    snprintf(path, sizeof(path),
             "/proc/%s/status", pid);

    file = fopen(path, "r");

    if (file == NULL)
    {
        printf("Name: unavailable\n");
        return;
    }

    while (fgets(line, sizeof(line), file) != NULL)
    {
        if (strncmp(line, "Name:", 5) == 0)
        {
            printf("%s", line);
            break;
        }
    }

    fclose(file);
}


/* ---------- Read process state and UID ---------- */

void read_process_info(const char *pid)
{
    char path[256];
    char line[256];

    FILE *file;

    snprintf(path, sizeof(path),
             "/proc/%s/status", pid);

    file = fopen(path, "r");

    if (file == NULL)
    {
        printf("Process information unavailable\n");
        return;
    }

    while (fgets(line, sizeof(line), file) != NULL)
    {
        if (strncmp(line, "State:", 6) == 0)
        {
            printf("%s", line);
        }

        if (strncmp(line, "Uid:", 4) == 0)
        {
            printf("%s", line);
        }
    }

    fclose(file);
}


/* ---------- Read memory usage ---------- */

void read_process_memory(const char *pid)
{
    char path[256];
    char line[256];

    FILE *file;

    snprintf(path, sizeof(path),
             "/proc/%s/status", pid);

    file = fopen(path, "r");

    if (file == NULL)
    {
        printf("Memory information unavailable\n");
        return;
    }

    while (fgets(line, sizeof(line), file) != NULL)
    {
        if (strncmp(line, "VmRSS:", 6) == 0)
        {
            printf("%s", line);
            break;
        }
    }

    fclose(file);
}


/* ---------- Read process CPU time ---------- */

unsigned long read_process_cpu(const char *pid)
{
    char path[256];
    char line[2048];

    FILE *file;

    unsigned long utime;
    unsigned long stime;

    int i;

    char *token;

    snprintf(path, sizeof(path),
             "/proc/%s/stat", pid);

    file = fopen(path, "r");

    if (file == NULL)
    {
        return 0;
    }

    if (fgets(line, sizeof(line), file) == NULL)
    {
        fclose(file);
        return 0;
    }

    fclose(file);

    /*
     * /proc/PID/stat contains many fields.
     *
     * Field 14 = user CPU time
     * Field 15 = kernel CPU time
     */

    token = strtok(line, " ");

    for (i = 1; i <= 13; i++)
    {
        if (token == NULL)
        {
            return 0;
        }

        token = strtok(NULL, " ");
    }

    if (token == NULL)
    {
        return 0;
    }

    utime = strtoul(token, NULL, 10);

    token = strtok(NULL, " ");

    if (token == NULL)
    {
        return 0;
    }

    stime = strtoul(token, NULL, 10);

    return utime + stime;
}


/* ---------- Read process I/O ---------- */

int read_process_io(const char *pid,
                    unsigned long *read_bytes,
                    unsigned long *write_bytes)
{
    char path[256];
    char line[256];

    FILE *file;

    *read_bytes = 0;
    *write_bytes = 0;

    snprintf(path, sizeof(path),
             "/proc/%s/io", pid);

    file = fopen(path, "r");

    if (file == NULL)
    {
        return 0;
    }

    while (fgets(line, sizeof(line), file) != NULL)
    {
        if (sscanf(line,
                   "read_bytes: %lu",
                   read_bytes) == 1)
        {
            continue;
        }

        if (sscanf(line,
                   "write_bytes: %lu",
                   write_bytes) == 1)
        {
            continue;
        }
    }

    fclose(file);

    return 1;
}


/* ---------- Read total system CPU time ---------- */

unsigned long long read_system_cpu(void)
{
    FILE *file;

    char line[1024];

    unsigned long long user;
    unsigned long long nice;
    unsigned long long system;
    unsigned long long idle;
    unsigned long long iowait;
    unsigned long long irq;
    unsigned long long softirq;
    unsigned long long steal;

    file = fopen("/proc/stat", "r");

    if (file == NULL)
    {
        return 0;
    }

    if (fgets(line, sizeof(line), file) == NULL)
    {
        fclose(file);
        return 0;
    }

    fclose(file);

    if (sscanf(line,
               "cpu %llu %llu %llu %llu %llu %llu %llu %llu",
               &user,
               &nice,
               &system,
               &idle,
               &iowait,
               &irq,
               &softirq,
               &steal) < 4)
    {
        return 0;
    }

    return user +
           nice +
           system +
           idle +
           iowait +
           irq +
           softirq +
           steal;
}


/* ---------- Main program ---------- */

int main(void)
{
    char pid[32];

    unsigned long process_cpu_before;
    unsigned long process_cpu_after;

    unsigned long long system_cpu_before;
    unsigned long long system_cpu_after;

    unsigned long process_cpu_delta;
    unsigned long long system_cpu_delta;

    double cpu_percentage;

    unsigned long read_before;
    unsigned long read_after;

    unsigned long write_before;
    unsigned long write_after;

    printf("Linux Process Analyzer\n");
    printf("=======================\n\n");

    printf("Enter PID to analyze: ");

    if (scanf("%31s", pid) != 1)
    {
        printf("Invalid input.\n");
        return 1;
    }

    printf("\nProcess Information for PID %s:\n", pid);

    read_process_name(pid);

    read_process_info(pid);

    read_process_memory(pid);

    process_cpu_before = read_process_cpu(pid);

    if (process_cpu_before == 0)
    {
        printf("CPU information may be unavailable.\n");
    }

    system_cpu_before = read_system_cpu();

    printf("CPU Time: %lu ticks\n",
           process_cpu_before);

    if (read_process_io(pid,
                        &read_before,
                        &write_before))
    {
        printf("Read Bytes : %lu\n",
               read_before);

        printf("Write Bytes: %lu\n",
               write_before);
    }
    else
    {
        printf("I/O information not available\n");
        read_before = 0;
        write_before = 0;
    }

    printf("\nMeasuring CPU activity for 1 second...\n");

    sleep(1);

    process_cpu_after = read_process_cpu(pid);

    system_cpu_after = read_system_cpu();

    process_cpu_delta =
        process_cpu_after - process_cpu_before;

    system_cpu_delta =
        system_cpu_after - system_cpu_before;

    printf("Process CPU activity: %lu ticks\n",
           process_cpu_delta);

    printf("System CPU activity: %llu ticks\n",
           system_cpu_delta);

    if (system_cpu_delta > 0)
    {
        cpu_percentage =
            ((double)process_cpu_delta /
             (double)system_cpu_delta) * 100.0;

        printf("Approximate CPU usage: %.2f%%\n",
               cpu_percentage);
    }
    else
    {
        printf("CPU usage: unavailable\n");
    }

    if (read_process_io(pid,
                        &read_after,
                        &write_after))
    {
        printf("\nI/O activity during interval:\n");

        printf("Read activity : %lu bytes\n",
               read_after - read_before);

        printf("Write activity: %lu bytes\n",
               write_after - write_before);
    }
    else
    {
        printf("\nI/O activity unavailable\n");
    }

    printf("\nRunning Process IDs:\n\n");

    {
        DIR *dir;
        struct dirent *entry;

        dir = opendir("/proc");

        if (dir == NULL)
        {
            printf("Unable to open /proc\n");
            return 1;
        }

        while ((entry = readdir(dir)) != NULL)
        {
            if (is_pid_directory(entry->d_name))
            {
                printf("PID: %s\n",
                       entry->d_name);
            }
        }

        closedir(dir);
    }

    return 0;
}
