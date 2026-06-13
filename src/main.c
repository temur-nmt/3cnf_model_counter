#include "header.h"

/* 
 * consumes the next formula from the input directory,
 * returns 1 if successful, 0 otherwise
 */
int consume_next_formula(Formula *phi) {
    // find files in input dir, if empty return false
    struct dirent *de;
    DIR *dr = opendir(INPUT_DIR);
    if (!dr)
        return 0;

    char path[512];
    int found = 0;
    while ((de = readdir(dr)) != NULL) {
        if (de->d_name[0] == '.')
            continue; // skip . and ..

        snprintf(path, sizeof path, "%s/%s", INPUT_DIR, de->d_name);
        found = 1;
        break;
    }
    closedir(dr);
    if (!found)
        return 0;

    // read file and parse into Formula struct, return false if invalid format
    FILE *fp = fopen(path, "r");
    if (!fp)
        return 0;

    char line[512];
    int lineno = 0;
    // read lines until end of formula, parse into clauses and add to phi
    while (fgets(line, sizeof(line), fp)) {
        lineno++;
        char *p = line;
        while (*p == ' ' || *p == '\t')
            p++;    // consume whitespace
        if (*p == '\n' || *p == '\r' || *p == '\0' || *p == 'c')
            continue;   // skip empty lines and comments
        if (*p == '%')
            break; // end of formula

        Clause c = {0};
        int ok = 1;
        char *tok = strtok(p, " \t\n\r");
        int c_index = 0;
        while (tok && c_index < ARITY) {
            Var v = atoi(tok);
            if (v == 0 || v > MAX_VARS) {
                ok = 0;
                break;
            }
            c[c_index] = lit_from_var(v);
            tok = strtok(NULL, " \t\n\r");
        }
        if (!ok) {
            fclose(fp);
            return 0;
        }
        append_clause(phi, c);
    }
    fclose(fp);
    return 1;
}

/* writes a formula to archive */
void archive_solution(Formula phi) {
    char path[512];
    snprintf(path, sizeof path, "%s/solution_%d.txt", OUTPUT_DIR, phi.count);
    FILE *fp = fopen(path, "w");
    if (!fp)
        return;

    // TODO: write top comment 
    for (int i = 0; i < phi.count; i++) {
        for (int j = 0; j < ARITY; j++) {
            fprintf(fp, "%d ", var_from_lit(phi.clauses[i][j]));
        }
        fprintf(fp, "\n");
    }
    // TODO: write solutions

    fclose(fp);
}

int main() {
    Formula phi = {0};
    while (consume_next_formula(&phi)) {
        // TODO: modelcount(phi);
        // TODO: archive_solution(phi);
    }
    return 1;
}