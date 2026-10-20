#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>

#define MAX_JOBS 20

typedef struct {
    int id;
    int arrival;
    int burst;
    int waiting;
    int turnaround;
    int done;
} Job;

static int read_int(const char *prompt, int min, int max)
{
    int value;
    for (;;) {
        printf("%s", prompt);
        if (scanf("%d", &value) == 1 && value >= min && value <= max) return value;
        puts("Invalid value. Please enter an allowed integer.");
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) {}
    }
}

static void print_results(const char *title, Job jobs[], int n, int order[])
{
    int i, time = 0;
    double total_wait = 0.0, total_turn = 0.0;
    printf("\n=== %s ===\n", title);
    puts("Gantt chart (process order; each job's duration is its burst time):");
    printf("0");
    for (i = 0; i < n; i++) {
        int idx = order[i];
        if (time < jobs[idx].arrival) {
            time = jobs[idx].arrival;
            printf(" -- idle -- %d", time);
        }
        time += jobs[idx].burst;
        printf(" -- P%d -- %d", jobs[idx].id, time);
    }
    puts("\n");
    puts("PID  Arrival  Burst  Waiting  Turnaround");
    for (i = 0; i < n; i++) {
        printf("P%-3d %-8d %-6d %-8d %d\n", jobs[i].id, jobs[i].arrival,
               jobs[i].burst, jobs[i].waiting, jobs[i].turnaround);
        total_wait += jobs[i].waiting;
        total_turn += jobs[i].turnaround;
    }
    printf("Average waiting time: %.2f\n", total_wait / n);
    printf("Average turnaround time: %.2f\n", total_turn / n);
}

static void run_fcfs(Job input[], int n)
{
    Job jobs[MAX_JOBS];
    int order[MAX_JOBS], i, j, time = 0;
    for (i = 0; i < n; i++) { jobs[i] = input[i]; order[i] = i; }
    for (i = 0; i < n; i++) {
        for (j = i + 1; j < n; j++) {
            if (jobs[order[j]].arrival < jobs[order[i]].arrival) {
                int tmp = order[i]; order[i] = order[j]; order[j] = tmp;
            }
        }
    }
    for (i = 0; i < n; i++) {
        int idx = order[i];
        if (time < jobs[idx].arrival) time = jobs[idx].arrival;
        jobs[idx].waiting = time - jobs[idx].arrival;
        time += jobs[idx].burst;
        jobs[idx].turnaround = time - jobs[idx].arrival;
    }
    print_results("FCFS (First Come, First Served)", jobs, n, order);
}

static void run_sjf(Job input[], int n)
{
    Job jobs[MAX_JOBS];
    int order[MAX_JOBS], i, done = 0, time = 0;
    for (i = 0; i < n; i++) { jobs[i] = input[i]; jobs[i].done = 0; }
    while (done < n) {
        int chosen = -1;
        for (i = 0; i < n; i++) {
            if (jobs[i].done || jobs[i].arrival > time) continue;
            if (chosen == -1 || jobs[i].burst < jobs[chosen].burst ||
                (jobs[i].burst == jobs[chosen].burst && jobs[i].arrival < jobs[chosen].arrival))
                chosen = i;
        }
        if (chosen == -1) {
            int next_arrival = -1;
            for (i = 0; i < n; i++)
                if (!jobs[i].done && (next_arrival == -1 || jobs[i].arrival < next_arrival))
                    next_arrival = jobs[i].arrival;
            time = next_arrival;
            continue;
        }
        jobs[chosen].waiting = time - jobs[chosen].arrival;
        time += jobs[chosen].burst;
        jobs[chosen].turnaround = time - jobs[chosen].arrival;
        jobs[chosen].done = 1;
        order[done++] = chosen;
    }
    print_results("SJF (non-preemptive)", jobs, n, order);
}

void run_scheduler_menu(void)
{
    Job jobs[MAX_JOBS];
    int n = read_int("Number of jobs (1-20): ", 1, MAX_JOBS);
    int i;
    puts("Enter arrival time and burst time for each job (both must be >= 0; burst > 0).");
    for (i = 0; i < n; i++) {
        char prompt[128];
        jobs[i].id = i + 1;
        snprintf(prompt, sizeof(prompt), "P%d arrival time: ", i + 1);
        jobs[i].arrival = read_int(prompt, 0, 1000000);
        snprintf(prompt, sizeof(prompt), "P%d burst time: ", i + 1);
        jobs[i].burst = read_int(prompt, 1, 1000000);
        jobs[i].done = 0;
    }
    run_fcfs(jobs, n);
    run_sjf(jobs, n);
}
