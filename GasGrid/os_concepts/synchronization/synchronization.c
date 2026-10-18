
#include <stdio.h>

int main(void) {
    int shared_counter = 0;
    int lock = 0;
    int choice;

    // GasGrid shared counter synchronization simulation
    printf("========== GASGRID SYNCHRONIZATION ==========\n");

    do {
        // Display the current counter and lock status
        printf("\nShared Booking Counter: %d\n", shared_counter);
        printf("Lock Status: %s\n", lock ? "Locked" : "Unlocked");

        printf("\n1. Enter Critical Section\n");
        printf("2. Read Shared Counter\n");
        printf("0. Exit\n");
        printf("Enter your choice: ");

        if (scanf("%d", &choice) != 1) {
            printf("Invalid input!\n");
            return 1;
        }

        // Simulate entering the critical section
        if (choice == 1) {
            if (lock == 1) {
                printf("Critical section unavailable. Try again later.\n");
            } else {
                // Acquire the simulated lock
                lock = 1;
                printf("Lock acquired successfully.\n");

                // Update the shared booking counter
                shared_counter++;
                printf("Booking counter incremented to %d.\n",
                       shared_counter);

                // Release the simulated lock
                lock = 0;
                printf("Lock released successfully.\n");
            }
        }

        // Read the shared counter
        else if (choice == 2) {
            printf("Current shared counter: %d\n", shared_counter);
        }

        // Handle invalid menu choices
        else if (choice != 0) {
            printf("Invalid choice! Please try again.\n");
        }

    } while (choice != 0);

    printf("\nGasGrid synchronization simulation completed.\n");

    return 0;
}
