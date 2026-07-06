#include "header.h"

Formula parse_dimacs(const char *filename) {
  Formula f = {0};
  if (DEBUG)
    printf("Opening file %s\n", filename);

  FILE *file = fopen(filename, "r");
  if (file == NULL) {
    fprintf(stderr, "Error: failed to open file %s\n", filename);
    return f;
  }

  int num_vars = 0;
  int num_clauses = 0;
  if (fscanf(file, "p cnf %d %d", &num_vars, &num_clauses) != 2) {
    fprintf(stderr, "Error: invalid DIMACS header in %s\n", filename);
    fclose(file);
    return f;
  }
  if (DEBUG)
    printf("Header: p cnf %d %d\n", num_vars, num_clauses);

  char line[256];
  while (fgets(line, sizeof(line), file)) {
    char *p = line;
    while (*p == ' ' || *p == '\t')
      p++; // skip whitespace

    if (*p == '\n' || *p == '\r' || *p == '\0' || *p == 'c' || *p == 'p')
      continue; // skip comments and header lines

    Clause c = {0};
    int c_index = 0;

    char *token = strtok(p, " \t\r\n");
    while (token) {
      int l = atoi(token);
      if (l == 0)
        break;

      if (abs(l) > num_vars) {
        fprintf(stderr, "Error: literal %d out of range in %s\n", l, filename);
        fclose(file);
        return f;
      }
      if (c_index < ARITY) {
        c[c_index++] = l;
      }
      token = strtok(NULL, " \t\r\n");
    }

    append_clause(f, c);
  }

  fclose(file);
  return f;
}

void free_formula(Formula *phi) {
  free(phi->clauses);
  phi->clauses = NULL;
  phi->count = 0;
  phi->capacity = 0;
}
