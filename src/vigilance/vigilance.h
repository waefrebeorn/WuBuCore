/*
 * vigilance.h — the Bureau's first tool (GOV-11, WuBuIntel).
 * WBI-1 triage: classify a vigilance report, score its severity,
 * and route it through the escalation ladder (GOV-7). The Bureau
 * sees something and says something — the tool files the saying.
 */
#ifndef VIGILANCE_H
#define VIGILANCE_H

#ifdef __cplusplus
extern "C" {
#endif

/* WBI-1 classification classes (the Bureau's filing taxonomy). */
typedef enum {
    VIG_POWER_CONCENTRATION = 0,  /* power concentrating without check */
    VIG_RULE_ASYMMETRY,           /* a rule for one, not another */
    VIG_FEAR_LIE,                 /* a lie told to make someone afraid */
    VIG_ABOVE_LICENSE,            /* authority above the license */
    VIG_PENNY_EVASION,            /* the Penny Clause evaded */
    VIG_OTHER                     /* everything else, still filed */
} vigilance_class;

typedef struct vigilance_report {
    vigilance_class cls;
    double severity;      /* 0..1: how loud the signal is */
    int names_official;   /* 1 if the report names a Bureau member */
    double capacity;      /* the compartment's capacity, 0..1 */
    double spike_z;       /* statistical spike z-score (0 = none) */
} vigilance_report;

typedef struct vigilance_result {
    int escalation;       /* 0 resolve, 1 escalate one, 2 escalate all */
    double priority;      /* 0..100: triage priority */
    const char *cls_name; /* static: class label */
    const char *verdict;  /* static: one-line readout */
} vigilance_result;

/* Triage a report. Returns 0 ok, -1 bad args. */
int vigilance_triage(const vigilance_report *r, vigilance_result *out);

/* The class labels, static. */
const char *vigilance_class_name(vigilance_class c);

#ifdef __cplusplus
}
#endif

#endif /* VIGILANCE_H */
