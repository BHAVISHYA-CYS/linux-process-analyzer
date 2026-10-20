# Testing plan

| Test | Action | Expected result |
|---|---|---|
| Build | `make clean && make` | Compiles with warnings enabled |
| PID analysis | Menu option 1, PID `1` | Displays available process metadata |
| Invalid PID | Menu option 1, PID `99999999` | Reports PID missing/unavailable |
| Process list | Menu option 2 | Lists numeric `/proc` entries |
| Security check | Menu option 3, PID `1` | Prints path and heuristic result or access warning |
| Security scan | Menu option 4 | Prints any path indicators and a summary |
| Scheduler | Menu option 5, 3 valid jobs | Prints FCFS and SJF tables and averages |

Actual results depend on current system processes and permissions. Record observed results after running the tests.
