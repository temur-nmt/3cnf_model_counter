/*
 * main.c
 *
 * Simple driver for the model-counting prototype. Scans `INPUT_DIR` for
 * DIMACS CNF files, parses them and runs both the sequential and parallel
 * brute-force counters. Output is intentionally concise to make it easy to
 * parse and compare timings.
 */

#include "header.h"
#include <omp.h>

/* Print a single clause in human-readable form. Used by debugging helpers. */
void print_clause(Clause c) {
  printf("[");
  for (int i = 0; i < ARITY - 1; i++) {
    printf("%d, ", c[i]);
  }
  printf("%d", c[ARITY - 1]);
  printf("]\n");
}

/* Print the whole formula (each clause on a single line). */
void print_formula(const Formula *f) {
  for (size_t i = 0; i < f->count; i++) {
    print_clause(f->clauses[i]);
  }
}

/* Entry point: enumerate files, parse, run counters and print results. */
int main() {
  FileQueue q = list_dir(INPUT_DIR);

  for (size_t i = 0; i < q.count; i++) {
    Formula f = parse_dimacs(q.items[i]);
    if (f.count == 0) {
      LOG_ERROR("skipping file %s due to parse errors or empty formula", q.items[i]);
      free_formula(&f);
      continue;
    }

    // Quick randomized satisfiability smoke-check (disabled by default).
    int *assignment = malloc((formula_num_vars(&f) + 1) * sizeof(int));
    if (assignment) {
      for (size_t j = 0; j < (size_t)(formula_num_vars(&f) + 1); j++)
        assignment[j] = rand() % 2;
      int result = check_formula(&f, assignment);
      (void)result; /* keep the call available for debugging */
      free(assignment);
    }

    int nvars = formula_num_vars(&f);
    printf("%s: formula has %zu clauses and %d variables\n", q.items[i], f.count, nvars);

    if (nvars <= 16) {
      double t0 = omp_get_wtime();
      unsigned long long seq_count = run_sequential_counting(&f);
      double t1 = omp_get_wtime();
      printf("%s: sequential count = %llu (time %0.6f s)\n", q.items[i], seq_count, t1 - t0);

      /* Implementation block: determine thread count from header option.
       * PARALLEL_THREADS == 0  -> auto (use omp_get_max_threads())
       * PARALLEL_THREADS == 1  -> sequential (1 thread)
       * PARALLEL_THREADS  > 1  -> use specified number of threads
       */
      int procs;
    #if PARALLEL_THREADS == 0
      procs = omp_get_max_threads();
    #elif PARALLEL_THREADS == 1
      procs = 1;
    #else
      procs = PARALLEL_THREADS;
    #endif
      double tp0 = omp_get_wtime();
      unsigned long long par_count = run_parallel_counting(&f, procs);
      double tp1 = omp_get_wtime();
      printf("%s: parallel count = %llu with %d threads (time %0.6f s)\n", q.items[i], par_count, procs, tp1 - tp0);
    } else {
      double th0 = omp_get_wtime();
      unsigned long long hybrid_count = count_formula(&f);
      double th1 = omp_get_wtime();
      printf("%s: hybrid count = %llu (time %0.6f s)\n", q.items[i], hybrid_count, th1 - th0);
    }

    free_formula(&f);
  }

  free(q.items);
  return 0;
}