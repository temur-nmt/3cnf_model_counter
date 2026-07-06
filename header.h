/*
 * header.h
 *
 * Shared definitions for the parallel-model-counting prototype.
 * This header exposes simple in-memory representations for DIMACS CNF
 * formulas, a small file-queue helper for enumerating input files, and
 * prototypes for parsing and counting functions used by the driver.
 *
 * Notes:
 * - This project is a lightweight research prototype intended for small
 *   SAT instances (current brute-force implementation uses 64-bit masks).
 */

#ifndef HEADER_H
#define HEADER_H

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Debug verbosity level (0 = quiet). Toggle for development only. */
#ifndef DEBUG
#define DEBUG 0
#endif

/* Logging macros used for controlled console output. */
#define LOG_ERROR(fmt, ...) fprintf(stderr, "ERROR: " fmt "\n", ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) \
  do { if (DEBUG) fprintf(stdout, fmt "\n", ##__VA_ARGS__); } while (0)
#define LOG_DEBUG(fmt, ...) \
  do { if (DEBUG > 1) fprintf(stderr, "DEBUG: " fmt "\n", ##__VA_ARGS__); } while (0)

/* Parallelization option:
 * - PARALLEL_THREADS == 0 : auto (use omp_get_max_threads())
 * - PARALLEL_THREADS == 1 : effectively sequential (no parallel worker spawning)
 * - PARALLEL_THREADS  > 1 : use this fixed number of threads for parallel counters
 */
#ifndef PARALLEL_THREADS
#define PARALLEL_THREADS 0
#endif

/* Project options */
#define INPUT_DIR "test_instances"
#define PATH_MAX_LEN 128
#define ARITY 3

/* Data structures ------------------------------------------------------- */
typedef int Lit;
typedef Lit Clause[ARITY];

/* Formula stores clauses as a dynamic array. Use append_clause to push.
 * The representation is intentionally simple for clarity. */
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

/* File queue: simple dynamic array of file path strings. */
typedef char Filename[PATH_MAX_LEN];
typedef struct {
  Filename *items;
  size_t count;
  size_t capacity;
} FileQueue;

/* Parser and I/O */
FileQueue list_dir(const char *path);
Formula parse_dimacs(const char *filename);
void free_formula(Formula *f);

/* Utilities */
int formula_num_vars(const Formula *f);

/* Model counting interfaces. Implementations exist in bruteforce.c and dpll.c */
int check_formula(Formula *f, int *assignment);
int generate_assignments(Formula *f, int *assignments, int current_var);
unsigned long long run_sequential_counting(Formula *f);
unsigned long long run_parallel_counting(Formula *f, int procs);

/* DPLL-based counter (future work) */
int run_dpll_counting(Formula *f, int procs);
/* Hybrid counting: decompose into components and count each component.
 * Uses brute force for small components (<=16 vars) and DPLL for larger ones.
 */
unsigned long long count_formula(Formula *f);

/* Helpers exposed for component extraction and counting */
unsigned long long bruteforce_count_component(const Formula *comp);
unsigned long long dpll_count_component(Formula *comp);

#endif // HEADER_H
