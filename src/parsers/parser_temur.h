#ifndef PARSER_H
#define PARSER_H

typedef struct {
    int literals[3];
} Clause;

typedef struct {
    int num_vars;
    int num_clauses;
    int expected_solution;
    Clause* clauses;
} Formula;

Formula parse_dimacs(const char* filename);
void free_formula(Formula* f);

#endif