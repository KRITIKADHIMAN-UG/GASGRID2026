
#include <stdio.h>

int main(void) {
    int choice, n, i, j;
    int burst[20], priority[20];
    int wait[20], turn[20];
    int process[20], remaining[20];
    int time, quantum, completed;
    int temp, current, shortest;
    int total_wait, total_turn;
    float avg_wait, avg_turn;

    do {
        // Display GasGrid CPU Scheduling Menu
        printf("\n========== GASGRID CPU SCHEDULING ==========\n");
        printf("1. First Come First Served (FCFS)\n");
        printf("2. Shortest Job First (SJF)\n");
        printf("3. Priority Scheduling\n");
        printf("4. Round Robin (RR)\n");
        printf("5. Shortest Remaining Time First (SRTF)\n");
        printf("6. Exit\n");
        printf("Enter your choice: ");

        if (scanf("%d", &choice) != 1) {
            printf("Invalid input!\n");
            return 1;
        }

        if (choice == 6) {
            printf("Exiting GasGrid CPU Scheduling Module.\n");
            break;
        }

        // Validate menu choice
        if (choice < 1 || choice > 5) {
            printf("Invalid choice! Try again.\n");
            continue;
        }

        // Input number of booking processes
        printf("Enter number of processes (1-20): ");

        if (scanf("%d", &n) != 1 || n < 1 || n > 20) {
            printf("Invalid number of processes!\n");
            return 1;
        }

        // Initialize process IDs and take burst times
        for (i = 0; i < n; i++) {
            process[i] = i + 1;

            printf("Enter burst time for P%d: ", i + 1);

            if (scanf("%d", &burst[i]) != 1 || burst[i] < 1) {
                printf("Invalid burst time!\n");
                return 1;
            }
        }

        // Initialize result variables
        total_wait = 0;
        total_turn = 0;
        avg_wait = 0;
        avg_turn = 0;

        // FCFS: Execute processes in arrival order
        if (choice == 1) {
            wait[0] = 0;

            for (i = 1; i < n; i++) {
                wait[i] = wait[i - 1] + burst[i - 1];
            }
        }

        // SJF: Execute shortest burst time first
        else if (choice == 2) {
            // Sort processes by burst time
            for (i = 0; i < n - 1; i++) {
                for (j = i + 1; j < n; j++) {
                    if (burst[i] > burst[j]) {
                        temp = burst[i];
                        burst[i] = burst[j];
                        burst[j] = temp;

                        temp = process[i];
                        process[i] = process[j];
                        process[j] = temp;
                    }
                }
            }

            // Calculate waiting times
            wait[0] = 0;

            for (i = 1; i < n; i++) {
                wait[i] = wait[i - 1] + burst[i - 1];
            }
        }

        // Priority Scheduling: Lower number means higher priority
        else if (choice == 3) {
            for (i = 0; i < n; i++) {
                printf("Enter priority for P%d (lower = higher priority): ",
                       process[i]);

                if (scanf("%d", &priority[i]) != 1) {
                    printf("Invalid priority!\n");
                    return 1;
                }
            }

            // Sort by priority, preserving process and burst details
            for (i = 0; i < n - 1; i++) {
                for (j = i + 1; j < n; j++) {
                    if (priority[i] > priority[j]) {
                        temp = priority[i];
                        priority[i] = priority[j];
                        priority[j] = temp;

                        temp = burst[i];
                        burst[i] = burst[j];
                        burst[j] = temp;

                        temp = process[i];
                        process[i] = process[j];
                        process[j] = temp;
                    }
                }
            }

            // Calculate waiting times
            wait[0] = 0;

            for (i = 1; i < n; i++) {
                wait[i] = wait[i - 1] + burst[i - 1];
            }
        }

        // Round Robin: Each process receives a fixed time quantum
        else if (choice == 4) {
            printf("Enter time quantum: ");

            if (scanf("%d", &quantum) != 1 || quantum < 1) {
                printf("Invalid time quantum!\n");
                return 1;
            }

            for (i = 0; i < n; i++) {
                remaining[i] = burst[i];
            }

            time = 0;
            completed = 0;

            // Execute processes in repeated circular order
            while (completed < n) {
                for (i = 0; i < n; i++) {
                    if (remaining[i] > 0) {
                        if (remaining[i] > quantum) {
                            time += quantum;
                            remaining[i] -= quantum;
                        } else {
                            time += remaining[i];
                            wait[i] = time - burst[i];
                            remaining[i] = 0;
                            completed++;
                        }
                    }
                }
            }
        }

        // SRTF: Preemptive scheduling using remaining burst time
        else if (choice == 5) {
            for (i = 0; i < n; i++) {
                remaining[i] = burst[i];
            }

            time = 0;
            completed = 0;

            // Select the process with the shortest remaining time
            while (completed < n) {
                shortest = -1;

                for (i = 0; i < n; i++) {
                    if (remaining[i] > 0 &&
                        (shortest == -1 ||
                         remaining[i] < remaining[shortest])) {
                        shortest = i;
                    }
                }

                // Run the selected process for one time unit
                if (shortest != -1) {
                    remaining[shortest]--;
                    time++;

                    // Record completion time and waiting time
                    if (remaining[shortest] == 0) {
                        turn[shortest] = time;
                        wait[shortest] =
                            turn[shortest] - burst[shortest];
                        completed++;
                    }
                }
            }
        }

        // Display results for all algorithms
        printf("\n========== SCHEDULING RESULTS ==========\n");
        printf("PID\tBurst\tWaiting\tTurnaround\n");

        for (i = 0; i < n; i++) {
            // SJF and Priority may reorder processes
            if (choice == 2 || choice == 3) {
                current = process[i];
            } else {
                current = i + 1;
            }

            // Turnaround time = Waiting time + Burst time
            if (choice != 5) {
                turn[i] = wait[i] + burst[i];
            }

            printf("P%d\t%d\t%d\t%d\n",
                   current, burst[i], wait[i], turn[i]);

            total_wait += wait[i];
            total_turn += turn[i];
        }

        // Calculate average scheduling times
        avg_wait = (float) total_wait / n;
        avg_turn = (float) total_turn / n;

        printf("\nAverage Waiting Time: %.2f\n", avg_wait);
        printf("Average Turnaround Time: %.2f\n", avg_turn);

    } while (choice != 6);

    return 0;
}
