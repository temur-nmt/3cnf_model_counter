#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define internal static
#define global_var static
#define local_persist static

#define ARITY 3
#define MAX_VARS 63 // for more typedef struct Var,Lit
#define INPUT_DIR "test_instances"
#define OUTPUT_DIR "solved"

/*
* --- Define Data Types for logical formulas ---
* Vars and Lits:
* bit 0 for sign,
* bits 1..63 for x_1..x_63
*
* Clauses have a fixed amount of Lits
* and are 0 if empty
*
* Formulas are dynamic arrays of Clauses
*/
typedef int Var;
typedef uint64_t Lit;
typedef Lit Clause[ARITY];
typedef struct {
    size_t count;
    size_t capacity;
    Clause *clauses;
} Formula;

// remove a clause by shifting the remainder forward by one
#define remove_clause(phi, index)                                            \
do {                                                                         \
    if (index > phi.count)                                                   \
    break;                                                                   \
    memmove(&phi.clauses[index], &phi.clauses[index + 1],                    \
        (phi.count - index - 1) * sizeof(*phi.clauses));                     \
        phi.count--;                                                         \
    } while (0)
    
// dynamic resizing macro for formulas
#define append_clause(phi, c)                                                    \
do {                                                                             \
    if (phi.count >= phi.capacity) {                                             \
        if (phi.capacity == 0)                                                   \
        phi.capacity = 256;                                                      \
        else                                                                     \
        phi.capacity *= 2;                                                       \
        phi.clauses = realloc(phi.clauses, phi.capacity * sizeof(*phi.clauses)); \
    }                                                                            \
    phi.clauses[phi.count++] = c;                                                \
} while (0)
    
/* --- helper functions --- */

/* for Vars -63..-1,1..63 returns the correct Literal, 0 otherwise */
inline Lit lit_from_var(Var v) {
    if (v == 0 || v > 63)
    return 0;
    return (v < 0) ? (1ULL << v) : ((1ULL << v) | 1);
}

/* returns the set bit at index k, ignores sign bit (0) */
inline Var var_from_lit(Lit l) {
    Var v = 0;
    do {
        l >>= 1;
        v++;
    } while (l);
    return v;
}

/* flips last bit of a literal and returns it */
inline Lit negegate(Lit l) { return l ^ 1; }

/* returns 1 if last bit is set, 0 otherwise */
inline int get_sign(Lit l) { return l & 1; }

/* if any literal in c == l, returns true, false otherwise */
inline Bool clause_contains_lit(Clause c, Lit l) {
    for (int i = 0; i < ARITY; i++) {
        if (c[i] == l)
        return true;
    }
    return false;
}

/* combines all clauses, then counts set bits */
inline int num_vars(Formula phi) {
    Lit vs = 0;
    for (int i = 0; i < phi.count; i++) {
        for (int j = 0; j < ARITY; j++) {
            vs |= phi.count[i][j];
        }
    }
    
    int n = 0;
    k &= ~1ULL;
    while (k) {
        k &= k - 1;
        n++;
    }
    return n;
}

/* combines all clauses, then creates monotone 1CNF */
inline Formula all_vars_from_formula(Formula phi) {
    Lit vs = 0;
    for (int i = 0; i < phi.count; i++) {
        for (int j = 0; j < ARITY; j++) {
            vs |= phi.count[i][j];
        }
    }
    
    Formula vars = {0};
    vs &= ~1ULL;
    while (vs) {
        Lit l = vs & -vs;
        vs &= vs - 1;
        append_clause(vars, l);
    }
    return vars;
}

/* returns all clauses that contain a literal */
inline Formula all_clauses_with_lit(Formula phi, Lit l) {
    Formula clauses = {0};
    for (int i = 0; i < phi.count; i++) {
        if (clause_contains_lit(phi.clauses[i], l)) {
            append_clause(clauses, phi.clauses[i]);
        }
    }
    return clauses;
}

/* provides deep copy of Formula struct */
inline Formula copy_formula(Formula phi) {
    Formula copy = {0};
    for (int i = 0; i < phi.count; i++) {
        append_clause(copy, phi.clauses[i]);
    }
    return copy;
}
