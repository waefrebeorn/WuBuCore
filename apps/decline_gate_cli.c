/* decline_gate_cli — run the population gate on one or more
 * decline signals (GOV-6). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "decline_gate.h"

int main(int argc, char **argv) {
    if (argc == 6 && strcmp(argv[1], "demo") == 0) {
        /* demo: baseline MEAN +/- STD, k, default */
        double mean = atof(argv[2]), std = atof(argv[3]);
        double k = atof(argv[4]);
        int def = atoi(argv[5]);
        char *labels[8] = {
            "usual (0.10)", "slight (0.13)", "raised (0.18)",
            "spike (0.30)",  "collapse (0.0)", "trend (0.09)",
            "trend+ (0.07)", "outlier (0.25)",
        };
        double sigs[8] = {0.10, 0.13, 0.18, 0.30, 0.00, 0.09, 0.07, 0.25};
        for (int i = 0; i < 8; i++) {
            dg_result r;
            if (decline_gate(mean, std, sigs[i], k, def, &r) != 0) {
                fprintf(stderr, "bad gate args\n");
                return 2;
            }
            printf("%-22s z=%+6.2f  %s  -> %s\n", labels[i], r.z,
                   r.anomaly ? "ANOMALY" : "clear  ",
                   r.process_on ? "ON (process happens)"
                                : "OFF (declined)");
        }
        return 0;
    }
    if (argc == 6) {
        double mean = atof(argv[1]), std = atof(argv[2]);
        double signal = atof(argv[3]), k = atof(argv[4]);
        int def = atoi(argv[5]);
        dg_result r;
        if (decline_gate(mean, std, signal, k, def, &r) != 0) {
            fprintf(stderr, "bad gate args\n");
            return 2;
        }
        printf("mean %.2f  std %.2f  signal %.2f  k %.1f  default %s\n",
               mean, std, signal, k, def ? "ON" : "off");
        printf("z        : %+.2f\n", r.z);
        printf("anomaly  : %s\n", r.anomaly ? "YES" : "no");
        printf("process  : %s\n", r.process_on ? "ON (happens)" :
                                                  "OFF (declined)");
        printf("reason   : %s\n", r.reason);
        return 0;
    }
    fprintf(stderr,
            "usage: decline_gate MEAN STD SIGNAL K DEFAULT_ON(0|1)\n"
            "       decline_gate demo MEAN STD K DEFAULT_ON\n");
    return 2;
}
