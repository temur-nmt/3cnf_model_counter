#ifndef SEQUENTIAL_H
#define SEQUENTIAL_H

#include "parser.h"

int run_sequential_counting(Formula* f);
int check_formula(Formula* f, int* assignment);
int generate_assignments(Formula* f, int* assignments, int current_var);

#endif