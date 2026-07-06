#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <omp.h>
#include "parser.h" 

/*
    Evaluates a clause.

    returns:
        * 1:    the clause is already satisfied
        * -1:   the clause is not satisfiable
        * 0:    we are not finished yet
*/
int evaluate_clause(Formula* f, int clause_idx, int* assignments, int* unit_lit) {
    int unassigned_count = 0;
    int last_unassigned_lit = 0;
    
    // check all literals
    for (int j = 0; j < 3; j++) {
        int lit = f->clauses[clause_idx].literals[j];
        
        int var = abs(lit);
        int is_pos = (lit > 0) ? 1 : 0;
        
        if (assignments[var] != -1) {
            // if literal is true => clause is true
            if (assignments[var] == is_pos) return 1;
        } else {
            unassigned_count++;
            last_unassigned_lit = lit;
        }
    }
    
    // if all are alredy assigned and the for loop didn't return, the clause must be UNSAT
    if (unassigned_count == 0) return -1;
    // if there are unsigned variables, we are not done
    if (unassigned_count == 1) {
        *unit_lit = last_unassigned_lit;
        return 0;
    }
    return 0; 
}

/*
    Evaluates the formula f by evaluating every clause.

    returns:
        * 1:    the formula is already satisfied and we found a mode
        * -1:   the formula is a contradiction and this branch cannot be satisfied
        * 0:    we are not finished yet   
*/
int evaluate_formula(Formula* f, int* assignments, int* unit_literal) {
    // assume SAT and no unit literals
    int all_satisfied = 1;
    *unit_literal = 0;

    for (int i = 0; i < f->num_clauses; i++) {
        int local_unit = 0;
        int status = evaluate_clause(f, i, assignments, &local_unit);
        
        // clause UNSAT => f UNSAT
        if (status == -1) return -1;
        if (status == 0) {
            all_satisfied = 0;
            // set clause as unit clause if none was set before
            if (*unit_literal == 0 && local_unit != 0) {
                *unit_literal = local_unit; 
            }
        }
    }
    return all_satisfied ? 1 : 0;
}

/*
    Applies unit propagation until:
        * a conclusion is reached (SAT/UNSAT)
        * there are no more unit clauses left

    returns:
        * 1:    the formula is already satisfied and we found a mode
        * -1:   the formula is a contradiction and this branch cannot be satisfied
        * 0:    we are not finished yet, but we have/not propagated
*/
int apply_unit_propagation(Formula* f, int* assignments, int* unassigned_vars) {
    int status, unit_literal;
    int changed = 1;

    while (changed == 1) {

        status = evaluate_formula(f, assignments, &unit_literal);
        // leave if the branch is dead or already SAT
        if (status == -1 || status == 1) {
            return status; 
        }
        
        if (unit_literal != 0) {
            int var = abs(unit_literal);
            if (unit_literal > 0) {
                // set to true
                assignments[var] = 1;
            } else {
                // set to false, as !0 = 1 
                assignments[var] = 0;
            }
            (*unassigned_vars)--; 
        } else {
            // no unit literals left
            changed = 0; 
        }
    }
    return 0;
}

/*
    Davis-Putnam_Logemann-Loveland. 

    * as long as there are unassigned variables propagate as long as you can.
    * reduce clauses by setting a variable to true/false until result is reached recursively
    
    returns:
        - amount of models found for an assignment
*/
uint64_t count_dpll(Formula* f, int* assignments, int unassigned_vars) {
    
    // copy of assignments for backtracking
    int local_assignments[f->num_vars + 1];
    memcpy(local_assignments, assignments, (f->num_vars + 1) * sizeof(int));
    
    int status = apply_unit_propagation(f, local_assignments, &unassigned_vars);
    
    // UNSAT, this Branch has no SAT
    if (status == -1) return 0;
    // SAT, all unassigned variables are also SAT
    if (status == 1) {
        return (uint64_t) pow(2, unassigned_vars);
    }
    // not finished, take next variable
    int next_var = -1;
    for (int i = 1; i <= f->num_vars; i++) {
        if (local_assignments[i] == -1) {
            next_var = i;
            break;
        }
    }
    // SAT: no more variables left
    if (next_var == -1) return 1; 
    
    uint64_t total = 0;
    // recursive dpll, set variable to false
    local_assignments[next_var] = 0;
    total += count_dpll(f, local_assignments, unassigned_vars - 1);
    
    // recursive dpll, set variable to true
    local_assignments[next_var] = 1;
    total += count_dpll(f, local_assignments, unassigned_vars - 1);
    
    return total;
}

/*
    HIGH LEVEL IDEA:

        1. Bound #x variables
        2. For each possible assignment of the variables call DPLL
        3. Collect all the results and sum them together

        By doing this parallel we can split the work into #procs (here 16) blocks.
        For this task each thread only has to call DPLL on 4 possible assignments of the first 6 Variables.
        DPLL avoids checking each and every possible assingment using unit propagation, delivering a possible models much faster.
        At the end all models for a certain assignment are collected by the thread and the overall total is summed.
*/

uint64_t run_parallel_counting(Formula* f, int procs) {
    
    // load balancing
    int tasks_per_thread = 4;
    int desired_tasks = procs * tasks_per_thread;
    int prefix_depth = (int)ceil(log2(desired_tasks));
    if (prefix_depth > f->num_vars) prefix_depth = f->num_vars;

    uint64_t total_tasks = (uint64_t) pow(2, prefix_depth); 
    uint64_t total_valid_models = 0;

    // use loop index to check all possible assignments of the first #prefix_depth variables
    #pragma omp parallel for num_threads(procs) reduction(+:total_valid_models)
    for (uint64_t i = 0; i < total_tasks; i++) {
        
        // assignments for the thread
        int* thread_assignments = (int*) malloc((f->num_vars + 1) * sizeof(int));
        if (thread_assignments == NULL) continue;

        // unbound variables are -1
        for (int v = 0; v <= f->num_vars; v++) {
            thread_assignments[v] = -1;
        }

        // create assignment based on index i
        uint64_t temp_i = i;
        for (int var = 1; var <= prefix_depth; var++) {
            int rest = temp_i % 2;
            thread_assignments[var] = rest;
            temp_i = temp_i / 2;
        }

        // let dpll figure out the model count
        int remaining_vars = f->num_vars - prefix_depth;
        uint64_t models_found = count_dpll(f, thread_assignments, remaining_vars);
        
        total_valid_models += models_found;
        free(thread_assignments);
    }

    return total_valid_models;
}
