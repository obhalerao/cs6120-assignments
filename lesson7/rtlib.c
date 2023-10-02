#include <stdio.h>
#include <stdlib.h>

int** log_data;

int num_funs = 5;
int num_loops = 5;

void start_logging() {
    log_data = malloc(num_funs * sizeof(int*));
    for (int i = 0; i < num_funs; i++) {
        log_data[i] = malloc(num_loops * sizeof(int));
        for (int j = 0; j < num_loops; j++) {
            log_data[i][j] = -1;
        }
    }
}

void logop(int fun_counter, int loop_counter) {
    if (fun_counter >= num_funs || loop_counter >= num_loops) {
        printf("[LOOP-COUNTER-DEBUG] called on %d and %d, but not logging. need more log space.\n", fun_counter, loop_counter);
        return;
    }
    if (log_data[fun_counter][loop_counter] == -1) {
        log_data[fun_counter][loop_counter] += 1;
    }
    log_data[fun_counter][loop_counter] += 1;
}

void end_logging() {
    for (int i = 0; i < num_funs; i++) {
        for (int j = 0; j < num_loops; j++) {
            if (log_data[i][j] == -1) continue;
            printf("function %d had its %dth loop header execute %d times\n", i, j, log_data[i][j]);
        }
        free(log_data[i]);
    }
    free(log_data);
}