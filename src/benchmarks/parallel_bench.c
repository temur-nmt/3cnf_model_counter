#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <omp.h>
#include "parallel.h"
#include "parser_temur.h"


/*
    THIS IS THE BENCH FOR PROBLEM SET B
    THE SEQUENTIAL ALGORITHM CAN'T HANDLE 63 VARIABLES, SO WE HAVE TO BENCH ISOLATED
*/

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("ERROR: NO PATH PROVIDED!\n");
        printf("USAGE: ./profiler <path_to_groupedCNF_folder>\n");
        return 1;
    }

    char* path = argv[1];
    struct timeval startTime, endTime;

    int procs = omp_get_num_procs();
    if (procs > 32) procs = 32; 
    omp_set_num_threads(procs);

    printf("PROFILING groupedCNF INSTANCES WITH %d THREADS\n", procs);
    printf("%-20s  %10s  %10s\n", "File", "Time (us)", "Models"); 
    printf("------------------------------------------------------------\n");

    for (int i = 1; i <= 20; i++) {
        char file_path[100];
        snprintf(file_path, sizeof(file_path), "%s/groupedCNF-%d.cnf", path, i);
        
        Formula f = parse_dimacs(file_path);

        gettimeofday(&startTime, NULL);
        int models = run_parallel_counting(&f, procs);
        gettimeofday(&endTime, NULL);
        
        long par_time = endTime.tv_usec - startTime.tv_usec + (endTime.tv_sec - startTime.tv_sec) * 1000000;

        printf("%-20s  %10ld  %10d\n", file_path, par_time, models);
        
        free(f.clauses); 
    }

    printf("------------------------------------------------------------\n");
    return 0;
}