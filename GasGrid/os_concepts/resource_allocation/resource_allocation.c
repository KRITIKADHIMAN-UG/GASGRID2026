
#include <stdio.h>

int main(void) {
    int total, allocated, request, available;

    // GasGrid resource allocation simulation
    printf("========== GASGRID RESOURCE ALLOCATION ==========\n");

    // Input the total number of available resource units
    printf("Enter total resource units: ");

    if (scanf("%d", &total) != 1 || total < 0) {
        printf("Invalid total resource count!\n");
        return 1;
    }

    // Input the number of resources already allocated
    printf("Enter currently allocated units: ");

    if (scanf("%d", &allocated) != 1 ||
        allocated < 0 || allocated > total) {
        printf("Invalid allocated resource count!\n");
        return 1;
    }

    // Calculate the resources currently available
    available = total - allocated;

    printf("\nTotal resources: %d\n", total);
    printf("Allocated resources: %d\n", allocated);
    printf("Available resources: %d\n", available);

    // Input the additional resources requested
    printf("\nEnter additional resource units requested: ");

    if (scanf("%d", &request) != 1 || request < 0) {
        printf("Invalid resource request!\n");
        return 1;
    }

    // Grant the request only if sufficient resources are available
    printf("\n========== ALLOCATION RESULT ==========\n");

    if (request <= available) {
        allocated += request;
        available -= request;

        printf("Request granted successfully.\n");
        printf("Updated allocated resources: %d\n", allocated);
        printf("Remaining available resources: %d\n", available);
    } else {
        printf("Request denied: insufficient available resources.\n");
        printf("Additional units required: %d\n", request - available);
        printf("Allocated resources remain unchanged: %d\n", allocated);
        printf("Available resources remain unchanged: %d\n", available);
    }

    printf("\nGasGrid resource allocation simulation completed.\n");

    return 0;
}
