# OS engine (C)

Six standalone programs in `os/src`, built to `os/bin`. Each reads text from **stdin** and writes **JSON** to stdout; the backend validates the client JSON, converts it and runs the program from a fixed whitelist (no shell). Errors come back as `{"error":"..."}`.

| Program | Input (stdin) | Covers |
|---|---|---|
| cpu_scheduling | `ALGO QUANTUM N` + N lines `pid arrival burst priority` | FCFS, SJF, SRTF, Priority (both), RR, MLQ, MLFQ; Gantt, completion/turnaround/waiting/response, averages, utilization, throughput |
| process_management | `N` + N lines `CMD pid burst priority resource` | PCB, states NEW/READY/RUNNING/WAITING/SUSPENDED/TERMINATED, create, terminate (releases allocation), suspend/resume, illegal transitions reported |
| synchronization | `race stock safe`, `sem init PVPV`, `pc cap PCPC`, `peterson n`, `tas\|cas\|none n`, `rw RRWR`, `dine n NAIVE\|ORDERED rounds` | race condition vs mutex, counting semaphore, producer-consumer, Peterson, TestAndSet, CompareAndSwap, readers-writers, dining philosophers |
| deadlock_detection | `bank n m ...`, `detect n m ...`, `rag n e ...` | Banker's safety + request check, detection, wait-for-graph cycle |
| memory_management | `fit FIRST\|BEST\|WORST ...`, `page ALGO frames n refs`, `table pagesize ...` | First/Best/Worst fit, FIFO, LRU, OPT, CLOCK (second chance), LFU, MFU, Belady's anomaly check, page-table translation |
| disk_scheduling | `ALGO head max up\|down n requests` | FCFS, SSTF, SCAN, C-SCAN, LOOK, C-LOOK, head path, total movement, comparison |

The concurrency programs are deterministic simulations of interleavings (no real threads), so results are repeatable. They model GasGrid concepts (stock counter, bookings, dispatcher) and never touch host OS resources.

**Not implemented in C:** user/kernel thread models and the access matrix (teaching panels live in the frontend only), fork/exec/signals lab, SMP/affinity, IPC, working set/thrashing, file-system and free-space management, swapping/NUMA.

Build: `make -C os` or `os\build.bat`. Test: `make -C os test`.
