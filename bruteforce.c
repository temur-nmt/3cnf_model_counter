/*
 * bruteforce.c
 *
 * Brute-force model counting helpers. The current implementation uses a
 * 64-bit mask enumeration (one bit per variable) to evaluate clauses
 * efficiently. The parallel version uses OpenMP to split the assignment
 * space between threads.
 */

#include "header.h"
#include <stdint.h>
#include <omp.h>

/* Check whether the formula is satisfied by the boolean assignment array
 * (0/1). The assignment array uses 0-based indexing for variables. */
int check_formula(Formula *f, int *assignment) {
  for (size_t i = 0; i < f->count; i++) {
    Clause *c = &f->clauses[i];
    int satisfied = 0;
    for (int j = 0; j < ARITY; j++) {
      int lit = (*c)[j];
      if (lit == 0)
        break;
      int var_index = abs(lit) - 1;
      int var_value = assignment[var_index];
      if ((lit > 0 && var_value == 1) || (lit < 0 && var_value == 0)) {
        satisfied = 1;
        break;
      }
    }
    if (!satisfied)
      return 0; // one clause f -> formula f
  }
  return 1; // all clauses true
}

// Determine number of variables used in the formula by scanning clauses.
int formula_num_vars(const Formula *f) {
  int maxv = 0;
  for (size_t i = 0; i < f->count; i++) {
    Clause *c = &f->clauses[i];
    for (int j = 0; j < ARITY; j++) {
      int lit = (*c)[j];
      if (lit == 0)
        break;
      int v = abs(lit);
      if (v > maxv)
        maxv = v;
    }
  }
  return maxv;
}

/* Optimized sequential brute-force counting using bitmasks. This avoids
 * per-assignment malloc/recursion and evaluates clauses using precomputed
 * positive and negative literal masks for each clause. */
unsigned long long run_sequential_counting(Formula *f) {
  int n = formula_num_vars(f);
  if (n <= 0)
    return 0;
  if (n > 63) {
    LOG_ERROR("run_sequential_counting: too many variables (%d) for bitmask brute force", n);
    return 0;
  }

  size_t m = f->count;
  uint64_t *pos = calloc(m, sizeof(uint64_t));
  uint64_t *neg = calloc(m, sizeof(uint64_t));
  if (!pos || !neg) {
    free(pos);
    free(neg);
    return 0;
  }

  for (size_t i = 0; i < m; i++) {
    Clause *c = &f->clauses[i];
    uint64_t p = 0;
    uint64_t nmask = 0;
    for (int j = 0; j < ARITY; j++) {
      int lit = (*c)[j];
      if (lit == 0)
        break;
      int idx = abs(lit) - 1;
      uint64_t bit = 1ULL << idx;
      if (lit > 0)
        p |= bit;
      else
        nmask |= bit;
    }
    pos[i] = p;
    neg[i] = nmask;
  }

  unsigned long long count = 0;
  uint64_t limit = (n == 64) ? ~0ULL : ((1ULL << n) - 1ULL);

  for (uint64_t assign = 0; assign <= limit; ++assign) {
    int ok = 1;
    for (size_t i = 0; i < m; i++) {
      // clause satisfied if any positive literal in assign OR any negative literal not in assign
      if (((pos[i] & assign) | (neg[i] & ~assign)) == 0) {
        ok = 0;
        break;
      }
    }
    if (ok)
      count++;
  }

  free(pos);
  free(neg);
  return count;
}

/*
    Counts valid Models for a Formula.

    params:
        -Formula* f:        Formula struct which defines the logic formula
    returns:
        - int:              amount of found Models
*/
/* Parallel enumeration using OpenMP. The function sets the OpenMP thread
 * count to `procs` (if > 0) and aggregates results using a per-thread local
 * counter that is atomically added to the global total at the end. */
unsigned long long run_parallel_counting(Formula *f, int procs) {
  int n = formula_num_vars(f);
  if (n <= 0)
    return 0;
  if (n > 62) {
    LOG_ERROR("run_parallel_counting: too many variables (%d) for 64-bit masked parallel brute force", n);
    return 0;
  }

  size_t m = f->count;
  uint64_t *pos = calloc(m, sizeof(uint64_t));
  uint64_t *neg = calloc(m, sizeof(uint64_t));
  if (!pos || !neg) {
    free(pos);
    free(neg);
    return 0;
  }

  for (size_t i = 0; i < m; i++) {
    Clause *c = &f->clauses[i];
    uint64_t p = 0;
    uint64_t nmask = 0;
    for (int j = 0; j < ARITY; j++) {
      int lit = (*c)[j];
      if (lit == 0)
        break;
      int idx = abs(lit) - 1;
      uint64_t bit = 1ULL << idx;
      if (lit > 0)
        p |= bit;
      else
        nmask |= bit;
    }
    pos[i] = p;
    neg[i] = nmask;
  }

  uint64_t limit = (n == 64) ? ~0ULL : ((1ULL << n) - 1ULL);
  unsigned long long total = 0;

  omp_set_num_threads(procs > 0 ? procs : 1);

#pragma omp parallel
  {
    unsigned long long local = 0;
#pragma omp for schedule(static)
    for (uint64_t assign = 0; assign <= limit; ++assign) {
      int ok = 1;
      for (size_t i = 0; i < m; i++) {
        if (((pos[i] & assign) | (neg[i] & ~assign)) == 0) {
          ok = 0;
          break;
        }
      }
      if (ok)
        local++;
    }
#pragma omp atomic
    total += local;
  }

  free(pos);
  free(neg);
  return total;
}