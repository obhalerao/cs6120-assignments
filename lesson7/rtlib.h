//
// Created by Sanjit Basker on 10/2/23.
//

#ifndef CS6120_ASSIGNMENTS_RTLIB_H
#define CS6120_ASSIGNMENTS_RTLIB_H

extern int** log_data;

extern int num_funs;
extern int num_loops;

void start_logging();
void logop(int fun_counter, int loop_counter);
void end_logging();


#endif //CS6120_ASSIGNMENTS_RTLIB_H
