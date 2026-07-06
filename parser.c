/*
 * parser.c
 *
 * Lightweight DIMACS CNF parser and simple directory enumerator used by the
 * driver. The parser tolerates leading comment lines (starting with 'c') and
 * requires a header line of the form: `p cnf <num-vars> <num-clauses>`.
 */

#include "header.h"

/* List non-hidden files in `path`. Returns a FileQueue with allocated
 * `items`. Caller must free `items` when done. On error an empty queue is
 * returned and an error is printed to stderr. */
FileQueue list_dir(const char *path) {
  FileQueue q = {0};
  DIR *dir = opendir(path);
  if (!dir) {
    LOG_ERROR("failed to open directory %s", path);
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
        LOG_ERROR("allocation of file queue items failed");
        break;
      }

      q.items = new_items;
      q.capacity = new_cap;
    }

    // build full path
    char full[PATH_MAX_LEN];
    int needed = snprintf(full, PATH_MAX_LEN, "%s/%s", path, ent->d_name);
    if (needed < 0 || needed >= PATH_MAX_LEN) {
      LOG_ERROR("path too long: %s/%s", path, ent->d_name);
      continue;
    }
    full[PATH_MAX_LEN - 1] = '\0';
    strncpy(q.items[q.count], full, PATH_MAX_LEN);
    q.items[q.count][PATH_MAX_LEN - 1] = '\0';
    LOG_INFO("%s/%s", path, ent->d_name);

    q.count++;
  }
  LOG_INFO("listed files successfully.");
  closedir(dir);
  return q;
}

Formula parse_dimacs(const char *filename) {
  Formula f = {0};
  LOG_INFO("Opening file %s", filename);

  FILE *file = fopen(filename, "r");
  if (file == NULL) {
    LOG_ERROR("failed to open file %s", filename);
    return f;
  }

  int num_vars = 0;
  int num_clauses = 0;
  int header_parsed = 0;

  char line[256];
  /* Read the file line-by-line. This is robust against comment lines and
   * avoids picking tokens from later positions in the stream. */
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
          LOG_ERROR("failed to read header line in %s", filename);
          fclose(file);
          return f;
        }
        header_parsed = 1;
        LOG_INFO("Header: %d variables, %d clauses", num_vars, num_clauses);
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

    /* Tokenize the clause line and collect up to ARITY literals. Literals
     * beyond ARITY are ignored in this simple prototype. */
    Clause c = {0};
    int c_index = 0;

    char *token = strtok(p, " \t\r\n");
    while (token) {
      int l = atoi(token);
      if (l == 0)
        break;

      if (abs(l) > num_vars) {
        LOG_ERROR("literal %d out of range in %s", l, filename);
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
    LOG_ERROR("missing header line in %s", filename);
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
