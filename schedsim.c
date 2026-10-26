
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include "process.h"
#include "util.h"

void findWaitingTimeRR(ProcessType plist[], int n, int quantum)
{
    int *rem_bt = malloc(n * sizeof(int));
    int *finished = calloc(n, sizeof(int));

    if (rem_bt == NULL || finished == NULL) {
        perror("Memory allocation failed");
        free(rem_bt);
        free(finished);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < n; i++) {
        rem_bt[i] = plist[i].bt;
        plist[i].wt = 0;
    }

    int completed = 0;
    int t = 0;

    while (completed < n) {
        int ran = 0;

        for (int i = 0; i < n; i++) {
            if (rem_bt[i] > 0 && plist[i].art <= t) {
                ran = 1;

                int slice = rem_bt[i] < quantum
                            ? rem_bt[i] : quantum;

                t += slice;
                rem_bt[i] -= slice;

                if (rem_bt[i] == 0) {
                    finished[i] = 1;
                    completed++;
                    plist[i].wt =
                        t - plist[i].art - plist[i].bt;

                    if (plist[i].wt < 0)
                        plist[i].wt = 0;
                }
            }
        }

        if (!ran)
            t++;
    }

    free(rem_bt);
    free(finished);
}

void findWaitingTimeSJF(ProcessType plist[], int n)
{
    int *rem_bt = malloc(n * sizeof(int));

    if (rem_bt == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < n; i++) {
        rem_bt[i] = plist[i].bt;
        plist[i].wt = 0;
    }

    int completed = 0;
    int t = 0;

    while (completed < n) {
        int shortest = -1;
        int min_remaining = INT_MAX;

        for (int i = 0; i < n; i++) {
            if (plist[i].art <= t &&
                rem_bt[i] > 0 &&
                rem_bt[i] < min_remaining) {
                min_remaining = rem_bt[i];
                shortest = i;
            }
        }

        if (shortest == -1) {
            t++;
            continue;
        }

        rem_bt[shortest]--;
        t++;

        if (rem_bt[shortest] == 0) {
            completed++;
            plist[shortest].wt =
                t - plist[shortest].art - plist[shortest].bt;

            if (plist[shortest].wt < 0)
                plist[shortest].wt = 0;
        }
    }

    free(rem_bt);
}

void findWaitingTime(ProcessType plist[], int n)
{
    int current_time = 0;

    for (int i = 0; i < n; i++) {
        if (current_time < plist[i].art)
            current_time = plist[i].art;

        plist[i].wt = current_time - plist[i].art;

        if (plist[i].wt < 0)
            plist[i].wt = 0;

        current_time += plist[i].bt;
    }
}

void findTurnAroundTime(ProcessType plist[], int n)
{
    for (int i = 0; i < n; i++)
        plist[i].tat = plist[i].wt + plist[i].bt;
}

int my_comparer(const void *this, const void *that)
{
    const ProcessType *a = (const ProcessType *)this;
    const ProcessType *b = (const ProcessType *)that;

    if (a->pri > b->pri)
        return -1;

    if (a->pri < b->pri)
        return 1;

    if (a->art < b->art)
        return -1;

    if (a->art > b->art)
        return 1;

    return 0;
}

void findavgTimeFCFS(ProcessType plist[], int n)
{
    findWaitingTime(plist, n);
    findTurnAroundTime(plist, n);
    printf("\n*********\nFCFS\n");
}

void findavgTimeSJF(ProcessType plist[], int n)
{
    findWaitingTimeSJF(plist, n);
    findTurnAroundTime(plist, n);
    printf("\n*********\nSJF\n");
}

void findavgTimeRR(ProcessType plist[], int n, int quantum)
{
    findWaitingTimeRR(plist, n, quantum);
    findTurnAroundTime(plist, n);
    printf("\n*********\nRR Quantum = %d\n", quantum);
}

void findavgTimePriority(ProcessType plist[], int n)
{
    qsort(plist, n, sizeof(ProcessType), my_comparer);
    findWaitingTime(plist, n);
    findTurnAroundTime(plist, n);
    printf("\n*********\nPriority\n");
}

void printMetrics(ProcessType plist[], int n)
{
    if (n <= 0) {
        printf("No processes to display.\n");
        return;
    }

    int total_wt = 0;
    int total_tat = 0;

    printf("\tProcesses\tBurst time\tWaiting time\tTurn around time\n");

    for (int i = 0; i < n; i++) {
        total_wt += plist[i].wt;
        total_tat += plist[i].tat;

        printf("\t%d\t\t%d\t\t%d\t\t%d\n",
               plist[i].pid, plist[i].bt,
               plist[i].wt, plist[i].tat);
    }

    printf("\nAverage waiting time = %.2f",
           (float)total_wt / n);
    printf("\nAverage turn around time = %.2f\n",
           (float)total_tat / n);
}

ProcessType *initProc(char *filename, int *n)
{
    FILE *input_file = fopen(filename, "r");

    if (!input_file) {
        fprintf(stderr, "Error: Invalid filepath\n");
        exit(EXIT_FAILURE);
    }

    ProcessType *plist = parse_file(input_file, n);
    fclose(input_file);

    return plist;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: ./schedsim <input-file-path>\n");
        return 1;
    }

    int n;
    int quantum = 2;
    ProcessType *proc_list;

    proc_list = initProc(argv[1], &n);
    findavgTimeFCFS(proc_list, n);
    printMetrics(proc_list, n);
    free(proc_list);

    proc_list = initProc(argv[1], &n);
    findavgTimeSJF(proc_list, n);
    printMetrics(proc_list, n);
    free(proc_list);

    proc_list = initProc(argv[1], &n);
    findavgTimePriority(proc_list, n);
    printMetrics(proc_list, n);
    free(proc_list);

    proc_list = initProc(argv[1], &n);
    findavgTimeRR(proc_list, n, quantum);
    printMetrics(proc_list, n);
    free(proc_list);

    return 0;
}
