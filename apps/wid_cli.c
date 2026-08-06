/* wid_cli — WID generate/validate. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wid.h"

int main(int argc, char **argv) {
    if (argc >= 2 && strcmp(argv[1], "generate") == 0 && argc == 5) {
        char out[WID_MAX];
        if (wid_generate(argv[2], atoi(argv[3]), atoi(argv[4]),
                         out, sizeof(out)) != 0) {
            fprintf(stderr, "bad input: country=2 letters, year 0-99, seq 0-9999\n");
            return 1;
        }
        printf("%s\n", out);
        return 0;
    }
    if (argc >= 2 && strcmp(argv[1], "validate") == 0 && argc == 3) {
        if (wid_validate(argv[2])) {
            printf("VALID\n");
            return 0;
        }
        printf("INVALID\n");
        return 1;
    }
    fprintf(stderr, "usage: wid generate <CC> <YY> <NNNN> | wid validate <WID>\n");
    return 2;
}
