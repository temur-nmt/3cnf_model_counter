#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <omp.h>
#include "sequential/sequential_bruteforce.h"

int check_formula_bitwise(Formula* f, uint64_t assignment) {
    
    for (int i = 0; i < f->num_clauses; i++) {
        int clause_is_true = 0;

        for (int j = 0; j < 3; j++) {
            int cur_literal = f->clauses[i].literals[j];
            int is_negative = (cur_literal < 0);
            
            int var_idx = abs(cur_literal) - 1; 
            
            int literal_assigned_value = (assignment >> var_idx) & 1;

            if ((!is_negative && literal_assigned_value == 1) || (is_negative && literal_assigned_value == 0)) {
                clause_is_true = 1;
                break;
            }
        }

        if (!clause_is_true) {
            return 0;
        }
    }
    return 1;
}