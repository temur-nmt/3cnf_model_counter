#ifndef PARALLEL_H
#define PARALLEL_H

#include "parser.h"
#include <stdint.h>

int evaluate_clause(Formula* f, int clause_idx, int* assignments, int* unit_lit);
int evaluate_formula(Formula* f, int* assignments, int* unit_literal);
int apply_unit_propagation(Formula* f, int* assignments, int* unassigned_vars);
uint64_t count_dpll(Formula* f, int* assignments, int unassigned_vars);
uint64_t run_parallel_counting(Formula* f, int procs);

#endif