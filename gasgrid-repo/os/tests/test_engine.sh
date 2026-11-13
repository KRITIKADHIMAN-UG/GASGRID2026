#!/bin/sh
# Smoke tests: known textbook answers. Run from os/ after building (make test).
B=${BIN:-bin}; F=0
chk() { if echo "$2" | grep -q "$3"; then echo "PASS $1"; else echo "FAIL $1: $2"; F=1; fi; }
chk "SRTF avg_waiting"  "$(printf 'SRTF 2 3\n1 0 5 1\n2 1 3 1\n3 2 4 1\n' | $B/cpu_scheduling)" '"avg_waiting":3.00'
chk "RR avg_waiting"    "$(printf 'RR 2 3\n1 0 5 1\n2 1 3 1\n3 2 4 1\n' | $B/cpu_scheduling)" '"avg_waiting":5.67'
chk "MLFQ runs"         "$(printf 'MLFQ 2 3\n1 0 5 1\n2 1 3 2\n3 2 4 3\n' | $B/cpu_scheduling)" '"throughput"'
chk "Banker safe seq"   "$(printf 'bank 5 3\n3 3 2\n7 5 3\n3 2 2\n9 0 2\n2 2 2\n4 3 3\n0 1 0\n2 0 0\n3 0 2\n2 1 1\n0 0 2\n-1 0 0 0\n' | $B/deadlock_detection)" '"sequence":\[1,3,4,0,2\]'
chk "Belady FIFO"      "$(printf 'page FIFO 3 12 1 2 3 4 1 2 5 1 2 3 4 5\n' | $B/memory_management)" '"fifo_faults":\[9,10\]'
chk "OPT faults"       "$(printf 'page OPT 3 12 1 2 3 4 1 2 5 1 2 3 4 5\n' | $B/memory_management)" '"OPT":7'
chk "Disk FCFS 640"    "$(printf 'FCFS 53 199 up 8 98 183 37 122 14 124 65 67\n' | $B/disk_scheduling)" '"total_movement":640'
chk "Disk SSTF 236"    "$(printf 'SSTF 53 199 up 8 98 183 37 122 14 124 65 67\n' | $B/disk_scheduling)" '"total_movement":236'
chk "Race oversell"    "$(printf 'race 1 0\n' | $B/synchronization)" '"oversold":true'
chk "Mutex safe"       "$(printf 'race 1 1\n' | $B/synchronization)" '"oversold":false'
chk "Peterson correct" "$(printf 'peterson 100\n' | $B/synchronization)" '"correct":true'
chk "No-lock loses"    "$(printf 'none 100\n' | $B/synchronization)" '"correct":false'
chk "Naive deadlock"   "$(printf 'dine 5 NAIVE 1\n' | $B/synchronization)" '"deadlock":true'
chk "Ordered no dl"    "$(printf 'dine 5 ORDERED 2\n' | $B/synchronization)" '"deadlock":false'
chk "PCB illegal"      "$(printf '2\nCREATE 1 5 1 STOCK\nDISPATCH 1 0 0 X\n' | $B/process_management)" '"ok":false'
exit $F
