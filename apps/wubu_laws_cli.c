/* wubu_laws_cli — the WuBu Laws of Tools compliance check. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wubu_laws.h"

int main(int argc, char **argv) {
    int flags[WUBU_LAW_COUNT] = {0, 0, 0, 0, 0};
    for (int i = 1; i + 1 < argc; i += 2) {
        int v = atoi(argv[i + 1]);
        if (strcmp(argv[i], "--no-harm") == 0) flags[WUBU_LAW_NO_HARM] = v;
        else if (strcmp(argv[i], "--filing-over-comfort") == 0) flags[WUBU_LAW_FILING_FIRST] = v;
        else if (strcmp(argv[i], "--can-say-no") == 0) flags[WUBU_LAW_CAN_SAY_NO] = v;
        else if (strcmp(argv[i], "--explicable") == 0) flags[WUBU_LAW_EXPLICABLE] = v;
        else if (strcmp(argv[i], "--no-companion") == 0) flags[WUBU_LAW_NO_COMPANION] = v;
    }
    printf("WUBU LAWS OF TOOLS - compliance check\n");
    printf("==============================================================\n");
    for (int i = 0; i < WUBU_LAW_COUNT; i++) {
        printf("  %s %s\n", flags[i] ? "[OK]" : "[XX]", wubu_law_text(i));
    }
    laws_result r = wubu_laws_check(flags);
    printf("==============================================================\n");
    printf("  %d/5 laws satisfied\n", r.satisfied);
    printf("  verdict: %s\n", r.verdict);
    return r.compliant ? 0 : 1;
}
