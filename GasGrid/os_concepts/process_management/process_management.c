
#include <stdio.h>

int main(void) {
    int n, i;
    int pid[20], burst[20];
    int state[20] = {0};

    // Input the number of GasGrid booking processes
    printf("========== GASGRID PROCESS MANAGEMENT ==========\n");
    printf("Enter number of booking processes (1-20): ");

    if (scanf("%d", &n) != 1 || n < 1 || n > 20) {
        printf("Invalid number of processes!\n");
        return 1;
    }

    // Assign process IDs and input processing times
    for (i = 0; i < n; i++) {
        pid[i] = i + 1;

        printf("Enter burst time for booking P%d: ", pid[i]);

        if (scanf("%d", &burst[i]) != 1 || burst[i] < 1) {
            printf("Invalid burst time!\n");
            return 1;
        }
    }

    // Display the initial process table
    printf("\n========== PROCESS TABLE ==========\n");
    printf("PID\tBurst Time\tState\n");

    for (i = 0; i < n; i++) {
        state[i] = 1;  // Mark process as ready

        printf("P%d\t%d\t\tReady\n", pid[i], burst[i]);
    }

    // Simulate process execution and completion
    printf("\n========== PROCESS EXECUTION ==========\n");

    for (i = 0; i < n; i++) {
        printf("Booking Process P%d is executing...\n", pid[i]);

        state[i] = 2;  // Mark process as running
        printf("P%d: Running\n", pid[i]);

        state[i] = 3;  // Mark process as completed
        printf("P%d: Completed\n", pid[i]);
    }

    // Display the final process status
    printf("\n========== FINAL PROCESS STATUS ==========\n");
    printf("PID\tBurst Time\tFinal State\n");

    for (i = 0; i < n; i++) {
        printf("P%d\t%d\t\t", pid[i], burst[i]);

        if (state[i] == 3) {
            printf("Completed\n");
        }
    }

    printf("\nTotal processes managed: %d\n", n);
    printf("GasGrid process management simulation completed.\n");

    return 0;
}
