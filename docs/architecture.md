# Architecture

- `src/main.c`: command-line menu and input handling.
- `src/process.c`: Linux `/proc` process information, CPU/I/O sampling and process listing.
- `src/security.c`: explainable executable-path indicator checks.
- `src/scheduler.c`: FCFS and non-preemptive SJF simulation.
- `include/*.h`: public function declarations.

The program is read-only with respect to processes. It does not terminate processes or change scheduling policy.
