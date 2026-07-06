/*
 * dpll.c
 *
 * Component decomposition and hybrid counting helpers. This file currently
 * implements a union-find based component extractor and a hybrid runner
 * that uses the fast brute-force counter for small components (<=16
 * variables) and falls back to a DPLL placeholder / sequential counter for
 * larger components. A full DPLL implementation is planned as the next
 * step.
 */

#include "header.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* --- small dynamic int-list helper --- */
typedef struct {
  int *items;
  size_t count;
  size_t cap;
} IntVec;

static void iv_init(IntVec *v) { v->items = NULL; v->count = v->cap = 0; }
static void iv_push(IntVec *v, int x) {
  if (v->count >= v->cap) {
    v->cap = v->cap ? v->cap * 2 : 8;
    v->items = realloc(v->items, v->cap * sizeof(int));
  }
  v->items[v->count++] = x;
}
static void iv_free(IntVec *v) { free(v->items); v->items = NULL; v->count = v->cap = 0; }

/* union-find over 1..n variables */
static int uf_find(int *p, int x) { return p[x] == x ? x : (p[x] = uf_find(p, p[x])); }
static void uf_union(int *p, int a, int b) {
  a = uf_find(p, a); b = uf_find(p, b); if (a != b) p[b] = a;
}

/* compute variable-connected components
 * returns array of length (maxvar+1) mapping variable -> component id (0..k-1)
 * sets *out_k to number of components and *out_maxvar to max variable index
 * Caller must free returned array. */
static int *compute_var_components(const Formula *f, int *out_k, int *out_maxvar) {
  int maxv = 0;
  for (size_t i = 0; i < f->count; i++) {
    for (int j = 0; j < ARITY; j++) {
      int lit = f->clauses[i][j]; if (!lit) break; int v = abs(lit); if (v > maxv) maxv = v;
    }
  }
  *out_maxvar = maxv;
  if (maxv == 0) { *out_k = 0; return NULL; }

  int *parent = malloc((maxv + 1) * sizeof(int));
  for (int i = 1; i <= maxv; i++) parent[i] = i;

  /* union variables that appear together in a clause */
  for (size_t i = 0; i < f->count; i++) {
    int vars[ARITY]; int cnt = 0;
    for (int j = 0; j < ARITY; j++) { int lit = f->clauses[i][j]; if (!lit) break; vars[cnt++] = abs(lit); }
    for (int a = 1; a < cnt; a++) uf_union(parent, vars[0], vars[a]);
  }

  /* compress and map roots to compact component ids */
  int *root = malloc((maxv + 1) * sizeof(int));
  for (int i = 1; i <= maxv; i++) root[i] = uf_find(parent, i);
  free(parent);

  IntVec roots_map; iv_init(&roots_map);
  int *comp_id = malloc((maxv + 1) * sizeof(int));
  for (int i = 1; i <= maxv; i++) comp_id[i] = -1;
  for (int i = 1; i <= maxv; i++) {
    int r = root[i];
    if (comp_id[r] == -1) {
      comp_id[r] = (int)roots_map.count;
      iv_push(&roots_map, r);
    }
    comp_id[i] = comp_id[r];
  }
  iv_free(&roots_map);
  free(root);

  *out_k = comp_id[ (comp_id ? 1 : 0) ]; /* dummy to silence static analyzers */
  /* compute actual number of distinct components */
  int maxcid = -1; for (int i = 1; i <= maxv; i++) if (comp_id[i] > maxcid) maxcid = comp_id[i];
  int k = (maxcid >= 0) ? (maxcid + 1) : 0;
  *out_k = k;
  return comp_id;
}

/* extract components: returns array of Formula of length k (caller frees each via free_formula)
 * Also outputs array comp_var_counts[k] giving number of distinct variables in each component. */
static Formula *extract_components(const Formula *f, int **comp_var_counts_out, int *out_k) {
  int k, maxv;
  int *var2comp = compute_var_components(f, &k, &maxv);
  if (!var2comp || k == 0) {
    *out_k = 0; if (var2comp) free(var2comp); *comp_var_counts_out = NULL; return NULL;
  }

  /* collect variables per component */
  IntVec *vars = malloc(k * sizeof(IntVec));
  for (int i = 0; i < k; i++) iv_init(&vars[i]);
  for (int v = 1; v <= maxv; v++) {
    int cid = var2comp[v]; if (cid >= 0) iv_push(&vars[cid], v);
  }

  /* create mapping global->local for each component */
  int **global2local = malloc(k * sizeof(int*));
  int *comp_var_counts = malloc(k * sizeof(int));
  for (int i = 0; i < k; i++) {
    comp_var_counts[i] = (int)vars[i].count;
    global2local[i] = calloc(maxv + 1, sizeof(int));
    for (int j = 0; j < (int)vars[i].count; j++) {
      int gv = vars[i].items[j]; global2local[i][gv] = j + 1; /* local indices 1..n */
    }
  }

  /* allocate component formulas */
  Formula *comps = calloc(k, sizeof(Formula));
  for (size_t i = 0; i < f->count; i++) {
    /* determine clause's component by first literal */
    int first_var = 0;
    for (int j = 0; j < ARITY; j++) { int lit = f->clauses[i][j]; if (!lit) break; first_var = abs(lit); break; }
    if (first_var == 0) continue; /* skip empty clause */
    int cid = var2comp[first_var];
    Clause c = {0};
    for (int j = 0; j < ARITY; j++) {
      int lit = f->clauses[i][j]; if (!lit) break;
      int gv = abs(lit); int local = global2local[cid][gv];
      int sign = (lit > 0) ? 1 : -1;
      c[j] = sign * local;
    }
    append_clause(comps[cid], c);
  }

  /* cleanup temps */
  for (int i = 0; i < k; i++) {
    free(global2local[i]); iv_free(&vars[i]);
  }
  free(global2local);
  free(vars);
  free(var2comp);

  *comp_var_counts_out = comp_var_counts;
  *out_k = k;
  return comps;
}

/* wrapper for using existing brute-force counter on a component */
unsigned long long bruteforce_count_component(const Formula *comp) {
  int n = formula_num_vars(comp);
  if (n <= 0) return 0;
  if (n > 63) { LOG_ERROR("bruteforce_count_component: component has %d vars (>63)", n); return 0; }
  /* reuse run_sequential_counting which expects a non-const pointer */
  Formula tmp = *comp; /* shallow copy is fine */
  return run_sequential_counting(&tmp);
}

/* placeholder dpll counter: for now fall back to sequential brute-force up to 63 vars */
unsigned long long dpll_count_component(Formula *comp) {
  int n = formula_num_vars(comp);
  if (n <= 63) {
    LOG_INFO("dpll_count_component: falling back to sequential mask counter for %d vars", n);
    return run_sequential_counting(comp);
  }
  LOG_ERROR("dpll_count_component: component too large (%d), DPLL not implemented", n);
  return 0;
}

/* Hybrid top-level: decompose formula, count components and multiply results.
 * Uses cutoff 16: components with <=16 vars use brute force; larger use DPLL/fallback. */
unsigned long long count_formula(Formula *f) {
  int k = 0;
  int *comp_var_counts = NULL;
  Formula *comps = extract_components(f, &comp_var_counts, &k);
  if (!comps || k == 0) {
    LOG_ERROR("count_formula: no components found or empty formula");
    if (comps) free(comps);
    return 0;
  }

  unsigned long long result = 1ULL;
  for (int i = 0; i < k; i++) {
    int n = comp_var_counts[i];
    unsigned long long c = 0;
    if (n <= 16) {
      c = bruteforce_count_component(&comps[i]);
    } else {
      c = dpll_count_component(&comps[i]);
    }
    result *= c;
    free_formula(&comps[i]);
  }
  free(comps);
  free(comp_var_counts);
  return result;
}
