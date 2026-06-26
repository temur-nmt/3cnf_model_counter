#include <stdio.h>
#include <stdlib.h>

#include "sequential/sequential_bruteforce.h"
#include "parallel/parallel_temur.h"

int main() {
    int test_run = 3;

    if (test_run == 0) {
        printf("TESTING SEQUENTIAL:\n");
        for (int i = 1; i <= 20; i++) {
            char path[40];
            snprintf(path, sizeof(path), "../test_instances/dummy_15vars_%d.cnf", i);
    
            Formula f = parse_dimacs(path);
            int count = run_sequential_counting(&f);
            if (count == f.expected_solution) {
                printf("SOLUTION CORRECT. FOUND %d MODELS\n", count);
            }
            else {
                printf("SOLUTION INCORRECT. EXPECTED %d MODELS, FOUND %d\n", f.expected_solution, count);
            }
        }
    } else if (test_run == 1) {
        printf("MODEL COUNTING SEQUENTIAL:\n");
        for (int i = 1; i <= 20; i++) {
            char path[40];
            snprintf(path, sizeof(path), "../Probleminstanzen_A/uf20-0%d.cnf", i);
    
            Formula f = parse_dimacs(path);
            int count = run_sequential_counting(&f);
            printf("FOUND %d MODELS\n", count);
        }
    } else if (test_run == 2) {
        printf("MODEL COUNTING PARALLEL:\n");
        for (int i = 1; i <= 20; i++) {
            char path[40];
            snprintf(path, sizeof(path), "../Probleminstanzen_A/uf20-0%d.cnf", i);
    
            Formula f = parse_dimacs(path);
            int count = run_parallel_counting(&f, 0);
            printf("FOUND %d MODELS\n", count);
        }
    } else if (test_run == 3) {
        printf("TESTING PARALLEL:\n");
        for (int i = 1; i <= 20; i++) {
            char path[40];
            snprintf(path, sizeof(path), "../test_instances/dummy_15vars_%d.cnf", i);
    
            Formula f = parse_dimacs(path);
            int count = run_parallel_counting(&f, 0);
            if (count == f.expected_solution) {
                printf("SOLUTION CORRECT. FOUND %d MODELS\n", count);
            }
            else {
                printf("SOLUTION INCORRECT. EXPECTED %d MODELS, FOUND %d\n", f.expected_solution, count);
            }
        }
    }
    return 0;
}