#include "process.h"
#include "security.h"
#include "scheduler.h"

#include <stdio.h>
#include <string.h>

static void clear_input_line(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

int main(void)
{
    char choice[32];
    for (;;) {
        puts("\n========================================");
        puts(" LINUX PROCESS ANALYZER");
        puts("========================================");
        puts("1. Analyze a process by PID");
        puts("2. List running process IDs and names");
        puts("3. Check one process for suspicious indicators");
        puts("4. Scan accessible processes for suspicious indicators");
        puts("5. CPU scheduling simulator (FCFS and non-preemptive SJF)");
        puts("0. Exit");
        printf("Choose an option: ");
        if (scanf("%31s", choice) != 1) return 1;
        if (strcmp(choice, "0") == 0) {
            puts("Exiting Linux Process Analyzer.");
            break;
        } else if (strcmp(choice, "1") == 0) {
            char pid[32];
            printf("Enter PID to analyze: ");
            if (scanf("%31s", pid) != 1) return 1;
            analyze_process(pid);
        } else if (strcmp(choice, "2") == 0) {
            list_processes();
        } else if (strcmp(choice, "3") == 0) {
            char pid[32];
            printf("Enter PID to check: ");
            if (scanf("%31s", pid) != 1) return 1;
            check_suspicious_process(pid);
        } else if (strcmp(choice, "4") == 0) {
            scan_processes_for_indicators();
        } else if (strcmp(choice, "5") == 0) {
            run_scheduler_menu();
        } else {
            puts("Unknown option. Choose 0 through 5.");
        }
        clear_input_line();
    }
    return 0;
}
