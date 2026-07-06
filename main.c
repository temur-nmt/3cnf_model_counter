#include "header.h"

void print_clause(Clause c) {
  printf("[");
  for (int i = 0; i < ARITY - 1; i++) {
    printf("%d, ", c[i]);
  }
  printf("%d", c[ARITY - 1]);
  printf("]\n");
}

void print_formula(const Formula *f) {
  for (size_t i = 0; i < f->count; i++) {
    print_clause(f->clauses[i]);
  }
}

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

int main() {
  FileQueue q = list_dir(INPUT_DIR);

  for (size_t i = 0; i < q.count; i++) {
    Formula f = parse_dimacs(q.items[i]);
    if (DEBUG) {
      print_formula(&f);
      printf("\n");
    }
    free_formula(&f);
  }

  free(q.items);
  return 0;
}

/*
if (TESTING) {
    if (DEBUG)
      printf("TESTING PARALLEL:\n");
  } else {
    if (DEBUG)
      printf("TESTING SEQUENTIAL:\n");
  }

  // get input dir
  struct dirent *de;
  DIR *dr = opendir(INPUT_DIR);
  if (!dr) {
    fprintf(stderr, "Error: could not open input directory %s\n", INPUT_DIR);
    return -1;
  }
  if (DEBUG)
    printf("input directory: %s\n", INPUT_DIR);

  // count all non-hidden files in input_dir
  char path[256];
  int found = 0;
  while ((de = readdir(dr)) != NULL) {
    if (de->d_name[0] == '.')
      continue; // skip hidden files, . and ..
    if (DEBUG)
      printf("%s\n", de->d_name);
    found++;
  }
  if (found == 0) {
    fprintf(stderr, "Error: no files found in input directory\n");
    closedir(dr);
    return -1;
  }
  closedir(dr);

  // parse file into Formula struct
  FILE *fp = fopen(path, "r");
  if (!fp) {
    fprintf(stderr, "Error: could not open input file %s\n", path);
    return -1;
  }


    Formula phi;
    phi.num_vars = 0;
    phi.num_clauses = 0;


fclose(fp);

// test print
for (int i = 0; i < max_clauses; i++) {
  printf("CLause %d: ", i);
  for (int j = 0; j < ARITY) {
    // print lits
    printf("%d ", phi[i][j]);
  }
}
*/
