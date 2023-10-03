#include <stdio.h>
#include "../lesson7/rtlib.h"

int main(int argc, const char** argv) {
    printf("enter a number and we'll do some operations on it.\n");
    start_logging();
    int num_iterations = 2;

    // add 2
    for (int i = 0; i < num_iterations; i++) {
        int num;
        scanf("%i", &num);
        printf("%i\n", num + 2);
    }

    // mul 2
    for (int i = 0; i < num_iterations; i++) {
        int num;
        scanf("%i", &num);
        printf("%i\n", num * 2);
    }

    // square
    for (int i = 0; i < num_iterations; i++) {
        int num;
        scanf("%i", &num);
        printf("%i\n", num * num);
    }

    // add 3
    for (int i = 0; i < num_iterations; i++) {
        int num;
        scanf("%i", &num);
        printf("%i\n", num + 3);
    }

    // mul 3
    for (int i = 0; i < num_iterations; i++) {
        int num;
        scanf("%i", &num);
        printf("%i\n", num * 3);
    }

    // cube
    for (int i = 0; i < num_iterations; i++) {
        int num;
        scanf("%i", &num);
        printf("%i\n", num * num * num);
    }

    end_logging();
    return 0;
}
