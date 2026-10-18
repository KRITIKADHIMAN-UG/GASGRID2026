
#include <stdio.h>

int main(void) {
    int n, m;
    int alloc[10][10], need[10][10];
    int avail[10], finish[10] = {0};
    int safe[10];
    int i, j, count = 0, progress;

    // Input the number of processes and resource types
    printf("========== GASGRID RESOURCE MANAGEMENT ==========\n");
    printf("Enter number of processes and resource types (max 10 each): ");

    if (scanf("%d %d", &n, &m) != 2 ||
        n < 1 || n > 10 || m < 1 || m > 10) {
        printf("Invalid number of processes or resources!\n");
        return 1;
    }

    // Input resources currently allocated to each process
    printf("\nEnter allocation matrix:\n");
    printf("(Resources currently allocated to each process)\n");

    for (i = 0; i < n; i++) {
        printf("Process P%d: ", i);

        for (j = 0; j < m; j++) {
            if (scanf("%d", &alloc[i][j]) != 1 ||
                alloc[i][j] < 0) {
                printf("Invalid allocation value!\n");
                return 1;
            }
        }
    }

    // Input remaining resource requirements of each process
    printf("\nEnter remaining-need matrix:\n");
    printf("(Additional resources required by each process)\n");

    for (i = 0; i < n; i++) {
        printf("Process P%d: ", i);

        for (j = 0; j < m; j++) {
            if (scanf("%d", &need[i][j]) != 1 ||
                need[i][j] < 0) {
                printf("Invalid need value!\n");
                return 1;
            }
        }
    }

    // Input the currently available resources
    printf("\nEnter available resources: ");

    for (j = 0; j < m; j++) {
        if (scanf("%d", &avail[j]) != 1 ||
            avail[j] < 0) {
            printf("Invalid available resource value!\n");
            return 1;
        }
    }

    // Display the resource allocation safety check
    printf("\n========== SAFETY CHECK ==========\n");

    // Find a process whose remaining needs can be satisfied
    do {
        progress = 0;

        for (i = 0; i < n; i++) {
            if (finish[i] == 0) {
                int can_execute = 1;

                // Check whether available resources meet its needs
                for (j = 0; j < m; j++) {
                    if (need[i][j] > avail[j]) {
                        can_execute = 0;
                        break;
                    }
                }

                // If possible, allow the process to finish
                if (can_execute) {
                    printf("Process P%d can complete.\n", i);

                    // Release its allocated resources
                    for (j = 0; j < m; j++) {
                        avail[j] += alloc[i][j];
                    }

                    finish[i] = 1;
                    safe[count++] = i;
                    progress = 1;
                }
            }
        }

    } while (progress == 1);

    // Display the final safety result
    if (count == n) {
        printf("\nSystem is in a SAFE state.\n");
        printf("Safe sequence: ");

        for (i = 0; i < n; i++) {
            printf("P%d", safe[i]);

            if (i < n - 1) {
                printf(" -> ");
            }
        }

        printf("\n");
    } else {
        printf("\nSystem is in an UNSAFE state.\n");
        printf("A safe sequence could not be found.\n");
        printf("Deadlock may occur.\n");
    }

    return 0;
}
