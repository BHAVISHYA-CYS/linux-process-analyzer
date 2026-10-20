# Methodology

## Process monitoring
Linux exposes process metadata in the virtual `/proc` filesystem. The program reads selected files such as `/proc/PID/status`, `/proc/PID/cmdline`, `/proc/PID/stat`, `/proc/PID/io`, and `/proc/PID/exe`.

## Suspicious indicators
The rule checker flags executable paths in commonly writable temporary/runtime locations and executable paths that end in `(deleted)`. These are investigation leads only. Legitimate software can use these paths, and malware can run from ordinary locations.

## CPU scheduling
The scheduler simulator accepts arrival and burst times. FCFS orders jobs by arrival time. Non-preemptive SJF chooses the shortest burst among jobs that have arrived when the CPU becomes free. Waiting time is start time minus arrival time; turnaround time is completion time minus arrival time.
