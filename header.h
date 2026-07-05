#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// syntactic sugar
#define internal static
#define global_var static
#define local_persist static

// debug params
#define DEBUG 1
#define PARALLEL 0

// basic options
#define INPUT_DIR "test_instances"
#define OUTPUT_DIR "test_instances"
#define PATH_MAX_LEN 128
#define ARITY 3
int max_clauses;
int max_vars;

// data types for modelcounting
typedef int Lit;
typedef Lit Clause[ARITY];

typedef struct {
  Clause *clauses;
  size_t count;
  size_t capacity;
} Formula;
#define append_clause(da, i)                                                   \
  do {                                                                         \
    if (da.count >= da.capacity) {                                             \
      if (da.capacity == 0) {                                                  \
        da.capacity = 256;                                                     \
      } else {                                                                 \
        da.capacity *= 2;                                                      \
      }                                                                        \
      da.clauses = realloc(da.clauses, da.capacity * sizeof(*da.clauses));     \
    }                                                                          \
    memcpy(da.clauses[da.count++], i, sizeof(Clause));                         \
  } while (0)

// helper types
typedef char Filename[PATH_MAX_LEN];
typedef struct {
  Filename *items;
  size_t count;
  size_t capacity;
} FileQueue;

// function declarations
Formula parse_dimacs(char *filename);
void free_formula(Formula *phi);

int run_parallel_counting(Formula *phi, int procs);

int run_sequential_counting(Formula *phi);
int check_formula(Formula *phi, int *assignment);
int generate_assignments(Formula *phi, int *assignments, int current_var);

// Passis dpll skizze
int generate_clause_groups(Formula *phi);
int check_for_merge(Formula *group, Clause c, int index);
int add_all_to_group(Clause c, Formula *group);
int merge_groups(Formula *groupA, Formula *groupB);
