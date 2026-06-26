#include <stdlib.h>
#include <stdio.h>
#include "sequential.h"
#include "parallel.h"
#include <omp.h>

/*
    Counts valid Models for a Formula.

    params:
        -Formula* f:        Formula struct which defines the logic formula
    returns:
        - int:              amount of found Models
*/
int run_parallel_counting(Formula* f, int procs) {
    
    int prefix_depth = 5; 
    
    if (f->num_vars < prefix_depth) {
        prefix_depth = f->num_vars;
    }

    int total_tasks = 1 << prefix_depth; 
    int total_valid_models = 0;

    #pragma omp parallel for num_threads(procs) reduction(+:total_valid_models)
    for (int i = 0; i < total_tasks; i++) {
        
        int* thread_assignments = (int*) calloc((f->num_vars + 1), sizeof(int));
        
        if (thread_assignments == NULL) {
            printf("ERROR: TTHREAD COULDNT ALLOCATE MEMORY.\n");
            continue;
        }

        for (int var = 1; var <= prefix_depth; var++) {
            int bit_is_set = (i >> (var - 1)) & 1; 
            thread_assignments[var] = bit_is_set;
        }

        int models_found = generate_assignments(f, thread_assignments, prefix_depth + 1);

        total_valid_models += models_found;

        free(thread_assignments);
    }

    return total_valid_models;
}

