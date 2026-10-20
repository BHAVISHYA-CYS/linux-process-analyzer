#ifndef SECURITY_H
#define SECURITY_H

void check_suspicious_process(const char *pid);
void scan_processes_for_indicators(void);

#endif
