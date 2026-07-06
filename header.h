#ifndef HEADER_H
#define HEADER_H

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// debug params
#define DEBUG 1
#define PARALLEL 0

// basic options
#define INPUT_DIR "test_instances"
#define PATH_MAX_LEN 128
#define ARITY 3

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

// parser interface
FileQueue list_dir(const char *path);
Formula parse_dimacs(const char *filename);
void free_formula(Formula *phi);

#endif // HEADER_H
