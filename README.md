# Linux Process Analyzer

A beginner-friendly Operating Systems project written in C for Linux.

## Features

- Reads process information from `/proc`: name, command line, state, PPID, UID, resident memory, thread count and start time.
- Samples per-process CPU time and I/O counters over one second where permissions allow.
- Lists process IDs and names.
- Flags selected suspicious executable-path indicators, including executables under `/tmp`, `/var/tmp`, `/dev/shm`, `/run/user`, or paths marked `(deleted)`.
- Simulates FCFS and non-preemptive SJF scheduling and reports waiting/turnaround times and averages.

## Build and run

```bash
make
./process_analyzer
```

To rebuild from scratch:

```bash
make clean
make
```

## Security limitations

This is an educational rule-based indicator checker, **not a full antivirus or malware scanner**. A path warning is not proof of malware, and no warning does not prove a process is safe. Some process details may be unavailable because a process exits while being inspected or because of Linux permissions. The program does not kill processes or modify them.

## Scheduling limitations

FCFS and non-preemptive SJF run as simulations over user-entered jobs. They do not change Linux's real scheduler or reschedule live processes.

## Demo checklist

1. Select option 1 and analyze PID `1` (if accessible).
2. Select option 2 to list processes.
3. Select option 3 and check PID `1` for a path indicator.
4. Select option 4 to scan accessible process executable paths.
5. Select option 5 and enter three jobs, for example arrival/burst pairs `(0, 5)`, `(1, 3)`, `(2, 1)`.
6. Capture screenshots of the menu, process analysis, security scan, and scheduler results.
