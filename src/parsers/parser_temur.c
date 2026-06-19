#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser_temur.h"


Formula parse_dimacs(const char* filename) {

    /*-------SETUP-------*/

    Formula f;
    f.num_vars = 0;
    f.num_clauses = 0;
    f.expected_solution = -1;
    f.clauses = NULL;

    /*-----SETUP-END-----*/

    FILE* file = fopen(filename,"r");
    if (!file) {
        printf("ERROR WHILE READING %s: FILE DOES NOT EXIST.", filename);
        exit(1);
    }

    char line[256];
    int reading_solution = 0;
    int clause_index = 0;

    while (fgets(line, sizeof(line), file)) {

        // check EOF and solution
        if (reading_solution == 1) {
            f.expected_solution = atoi(line);
            break;
        }

        // identify EOF
        if (strncmp(line, "solution", 8) == 0) {
            reading_solution = 1;
            continue;
        }

        // ignore comments
        if (line[0] == 'c') {
            continue;
        }

        // read in the metadata
        if (line[0] == 'p') {
            sscanf(line, "p cnf %d %d", &f.num_vars, &f.num_clauses);
            f.clauses = (Clause*)malloc(f.num_clauses * sizeof(Clause));
            continue;
        }

        // read in clauses
        if (f.clauses != NULL && clause_index < f.num_clauses) {
            int a1, a2, a3, zero;

            if (sscanf(line, "%d %d %d %d", &a1, &a2, &a3, &zero) >= 3) {
                f.clauses[clause_index].literals[0] = a1;
                f.clauses[clause_index].literals[1] = a2;
                f.clauses[clause_index].literals[2] = a3;
                clause_index++;
            }
        }

    }

    fclose(file);
    return f;
}

// cleanup function
void free_formula(Formula* f) {
    if (f->clauses != NULL) {
        free(f->clauses);
    }
}
