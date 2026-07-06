#include "header.h"

FileQueue list_dir(const char *path) {
  FileQueue q = {0};
  DIR *dir = opendir(path);
  if (!dir) {
    fprintf(stderr, "failed to open directory %s\n", path);
    return q;
  }

  struct dirent *ent;

  while ((ent = readdir(dir)) != NULL) {
    if (ent->d_name[0] == '.')
      continue;

    // grow if needed
    if (q.count >= q.capacity) {
      size_t new_cap = q.capacity ? q.capacity *= 2 : 16;

      Filename *new_items = realloc(q.items, new_cap * sizeof(Filename));
      if (!new_items) {
        fprintf(stderr, "Error: Allocation of new items failed\n");
        break;
      }

      q.items = new_items;
      q.capacity = new_cap;
    }

    // build full path
    char full[PATH_MAX_LEN];
    snprintf(full, PATH_MAX_LEN, "%s/%s", path, ent->d_name);
    full[PATH_MAX_LEN - 1] = '\0';
    strncpy(q.items[q.count], full, PATH_MAX_LEN);
    q.items[q.count][PATH_MAX_LEN - 1] = '\0';
    if (DEBUG)
      printf("%s/%s\n", path, ent->d_name);

    q.count++;
  }
  if (DEBUG)
    printf("listed files successfully.\n");
  closedir(dir);
  return q;
}

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
  int header_parsed = 0;

  char line[256];
  while (fgets(line, sizeof(line), file)) {
    char *p = line;
    while (*p == ' ' || *p == '\t')
      p++; // skip leading whitespace

    if (*p == '\n' || *p == '\r' || *p == '\0')
      continue;

    if (*p == 'c')
      continue; // skip comments

    if (!header_parsed) {
      if (*p == 'p') {
        if (sscanf(p, "p cnf %d %d", &num_vars, &num_clauses) != 2) {
          fprintf(stderr, "Error: failed to read header line in %s\n", filename);
          fclose(file);
          return f;
        }
        header_parsed = 1;
        if (DEBUG)
          printf("Header: %d variables, %d clauses\n", num_vars, num_clauses);
        continue;
      }
      continue;
    }

    if (*p == 'p') {
      continue; // ignore any later header lines
    }

    if (*p == '%') {
      break; // end of clauses
    }

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

  if (!header_parsed) {
    fprintf(stderr, "Error: missing header line in %s\n", filename);
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
