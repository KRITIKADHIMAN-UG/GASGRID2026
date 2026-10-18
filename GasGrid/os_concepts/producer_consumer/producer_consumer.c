
#include <stdio.h>

#define SIZE 5

int main(void) {
    int buffer[SIZE];
    int in = 0, out = 0, count = 0;
    int choice, value, i;

    // GasGrid shared buffer for LPG booking requests
    printf("========== GASGRID PRODUCER-CONSUMER ==========\n");
    printf("Shared buffer capacity: %d booking requests\n", SIZE);

    do {
        // Display the producer-consumer menu
        printf("\n1. Produce Booking Request\n");
        printf("2. Consume Booking Request\n");
        printf("3. Display Buffer\n");
        printf("0. Exit\n");
        printf("Enter your choice: ");

        if (scanf("%d", &choice) != 1) {
            printf("Invalid input!\n");
            return 1;
        }

        // Produce: Add a booking request to the buffer
        if (choice == 1) {
            if (count == SIZE) {
                printf("Buffer is full. Producer must wait.\n");
            } else {
                printf("Enter booking request ID: ");

                if (scanf("%d", &value) != 1) {
                    printf("Invalid booking ID!\n");
                    return 1;
                }

                buffer[in] = value;

                // Move to the next circular buffer position
                in = (in + 1) % SIZE;
                count++;

                printf("Booking request %d produced successfully.\n", value);
            }
        }

        // Consume: Remove the oldest booking request
        else if (choice == 2) {
            if (count == 0) {
                printf("Buffer is empty. Consumer must wait.\n");
            } else {
                printf("Booking request %d consumed.\n", buffer[out]);

                // Move to the next request in FIFO order
                out = (out + 1) % SIZE;
                count--;
            }
        }

        // Display all pending requests in FIFO order
        else if (choice == 3) {
            printf("\n========== PENDING BOOKING REQUESTS ==========\n");
            printf("Buffer capacity: %d\n", SIZE);
            printf("Pending requests: %d\n", count);

            if (count == 0) {
                printf("No pending booking requests.\n");
            } else {
                printf("Request IDs: ");

                for (i = 0; i < count; i++) {
                    printf("%d ", buffer[(out + i) % SIZE]);
                }

                printf("\n");
            }
        }

        // Handle invalid menu options
        else if (choice != 0) {
            printf("Invalid choice! Please try again.\n");
        }

    } while (choice != 0);

    printf("GasGrid Producer-Consumer simulation ended.\n");

    return 0;
}
