#include <stdlib.h>
#include <stdio.h>
#include "sequential_bruteforce.h"

/*
    Checks if an assignment satisfies the given Formula.

    params:
        -Formula* f:        Formula struct which defines the logic formula
        -int* assigment:    Array of assignments for the atoms, to avoid off-by-one assingment[0] is a placeholder
    returns:
        - 1: true => the assignment fullfills the Formula
        - 0: false => the assignment does not fullfill the Formula
*/
int check_formula(Formula* f, int* assignment) {
    
    for (int i = 0; i < f->num_clauses; i++) {

        int clause_is_true = 0;

        for (int j = 0; j < 3; j++) {

            int cur_literal = f->clauses[i].literals[j];
            int is_negative = (cur_literal < 0);
            int literal_assigned_value = assignment[abs(cur_literal)];

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
/*
    Generates all possible assignments of a Formula (2^n) and checks whether they are a model.

    params:
        -Formula* f:        Formula struct which defines the logic formula
        -int* assigment:    Array of assignments for the atoms, to avoid off-by-one assingment[0] is a placeholder
        -int current_val:   current variable which is set to 0 and 1
    returns:
        -valid_models:      Amount of valid models for a Formula   
*/
int generate_assignments(Formula* f, int* assignments, int current_var) {
    
    if (current_var > f-> num_vars) {
        return check_formula(f, assignments);
    }

    int valid_models = 0;

    assignments[current_var] = 0;
    valid_models += generate_assignments(f, assignments, current_var + 1);

    assignments[current_var] = 1;
    valid_models += generate_assignments(f, assignments, current_var + 1);

    return valid_models;
}

/*
    Counts valid Models for a Formula.

    params:
        -Formula* f:        Formula struct which defines the logic formula
    returns:
        - int: amount of found Models
*/
int run_sequential_counting(Formula* f) {
    int* assignments = (int*) malloc((f->num_vars + 1) * sizeof(int));
    if (assignments == NULL) {
        printf("ERROR OCCURED WHILE INITIALIZING assignments");
        return -1;
    }

    int total = generate_assignments(f,assignments,1);
    free(assignments);
    return total;
}

