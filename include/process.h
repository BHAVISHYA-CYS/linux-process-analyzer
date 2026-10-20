#ifndef PROCESS_H
#define PROCESS_H

void analyze_process(const char *pid);
void list_processes(void);
int is_pid_directory(const char *name);
unsigned long read_process_cpu(const char *pid);
unsigned long long read_system_cpu(void);

#endif
