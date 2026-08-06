/* pennychain_cli — mint / verify / root. Rows come from a CSV. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pennychain.h"
#include "wubusha256.h"

static int parse_rows(const char *path, penny_row **rows_out, size_t *n_out) {
    FILE *fp = fopen(path, "r");
    if (!fp) return -1;
    char line[1024];
    size_t cap = 0, n = 0;
    penny_row *rows = NULL;
    if (!fgets(line, sizeof(line), fp)) { /* header */ }
    while (fgets(line, sizeof(line), fp)) {
        if (n == cap) {
            cap = cap ? cap * 2 : 8;
            penny_row *nr = realloc(rows, cap * sizeof(penny_row));
            if (!nr) { free(rows); fclose(fp); return -1; }
            rows = nr;
        }
        /* penny_id,received_date,sender_wid,sender_name,project,type,... */
        penny_row *r = &rows[n];
        memset(r, 0, sizeof(*r));
        char *p = line;
        char *fields[6];
        for (int i = 0; i < 6; i++) {
            fields[i] = p;
            char *comma = strchr(p, ',');
            if (comma) { *comma = '\0'; p = comma + 1; }
        }
        snprintf(r->penny_id, sizeof(r->penny_id), "%s", fields[0]);
        snprintf(r->received_date, sizeof(r->received_date), "%s", fields[1]);
        snprintf(r->sender_wid, sizeof(r->sender_wid), "%s", fields[2]);
        snprintf(r->sender_name, sizeof(r->sender_name), "%s", fields[3]);
        snprintf(r->project, sizeof(r->project), "%s", fields[4]);
        snprintf(r->type, sizeof(r->type), "%s", fields[5]);
        /* trim newline from type */
        size_t tl = strlen(r->type);
        while (tl > 0 && (r->type[tl - 1] == '\n' || r->type[tl - 1] == '\r'))
            r->type[--tl] = '\0';
        n++;
    }
    fclose(fp);
    *rows_out = rows;
    *n_out = n;
    return 0;
}

int main(int argc, char **argv) {
    if (argc >= 3 && strcmp(argv[1], "mint") == 0) {
        penny_row *rows = NULL;
        size_t n = 0;
        if (parse_rows(argv[2], &rows, &n) != 0) {
            fprintf(stderr, "cannot read %s\n", argv[2]);
            return 1;
        }
        pennychain *pc = pennychain_new();
        if (!pc || pennychain_mint(pc, rows, n) != 0) {
            fprintf(stderr, "mint failed\n");
            free(rows);
            return 1;
        }
        char root[WUBU_SHA256_HEX];
        pennychain_latest_root(pc, root, sizeof(root));
        printf("MINTED %zu rows; root %s\n", n, root);
        free(rows);
        pennychain_free(pc);
        return 0;
    }
    if (argc >= 4 && strcmp(argv[1], "verify") == 0) {
        penny_row *rows = NULL;
        size_t n = 0;
        if (parse_rows(argv[2], &rows, &n) != 0) {
            fprintf(stderr, "cannot read %s\n", argv[2]);
            return 1;
        }
        pennychain *pc = pennychain_new();
        pennychain_mint(pc, rows, n);
        int ok = pennychain_verify(pc, argv[3]);
        printf("%s: %s\n", argv[3], ok == 1 ? "PROVEN" : "NOT FOUND");
        free(rows);
        pennychain_free(pc);
        return ok == 1 ? 0 : 1;
    }
    fprintf(stderr, "usage: pennychain mint <rows.csv> | pennychain verify <rows.csv> <id>\n");
    return 2;
}
