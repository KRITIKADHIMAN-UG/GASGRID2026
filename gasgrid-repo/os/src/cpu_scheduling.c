/* GasGrid OS engine: CPU scheduling.
   stdin : ALGO QUANTUM N, then N lines "pid arrival burst priority"
   ALGO  : FCFS SJF SRTF PRIORITY PRIORITY_P RR MLQ MLFQ
   stdout: JSON. MLQ levels by priority (1 urgent service, 2 refill, 3+ reporting), quanta 2/4/FCFS. */
#include <stdio.h>
#include <string.h>
#define M 64
#define TMAX 20000
typedef struct { int pid, at, bt, pr, rem, st, ct, lvl; } P;
static P p[M];
static int n, Q[3][M], qn[3], gp[TMAX], gs[TMAX], ge[TMAX], gn;
static void seg(int pid, int s, int e) {
    if (gn && gp[gn-1] == pid && ge[gn-1] == s) { ge[gn-1] = e; return; }
    gp[gn] = pid; gs[gn] = s; ge[gn] = e; gn++;
}
static void push(int l, int i) { Q[l][qn[l]++] = i; }
static void front(int l, int i) { memmove(&Q[l][1], &Q[l][0], qn[l] * sizeof(int)); Q[l][0] = i; qn[l]++; }
static int pop(int l) { int v = Q[l][0]; qn[l]--; memmove(&Q[l][0], &Q[l][1], qn[l] * sizeof(int)); return v; }
static int fail(const char *m) { printf("{\"error\":\"%s\"}\n", m); return 1; }
int main(void) {
    char a[16]; int q;
    if (scanf("%15s %d %d", a, &q, &n) != 3 || n < 1 || n > M || q < 1) return fail("bad input");
    for (int i = 0; i < n; i++) {
        if (scanf("%d %d %d %d", &p[i].pid, &p[i].at, &p[i].bt, &p[i].pr) != 4 || p[i].bt < 1 || p[i].at < 0) return fail("bad process");
        p[i].rem = p[i].bt; p[i].st = -1; p[i].ct = -1;
    }
    const char *ok[] = {"FCFS","SJF","SRTF","PRIORITY","PRIORITY_P","RR","MLQ","MLFQ"}; int valid = 0;
    for (int i = 0; i < 8; i++) if (!strcmp(a, ok[i])) valid = 1;
    if (!valid) return fail("unknown algorithm");
    int rr = !strcmp(a,"RR"), mlq = !strcmp(a,"MLQ"), mlfq = !strcmp(a,"MLFQ"), ml = mlq || mlfq, queued = rr || ml;
    int pre = !strcmp(a,"SRTF") || !strcmp(a,"PRIORITY_P");
    int qt[3] = { ml ? 2 : q, ml ? 4 : 1 << 30, 1 << 30 }, seen[M] = {0}, t = 0, cur = -1, used = 0, done = 0;
    for (int i = 0; i < n; i++) p[i].lvl = mlq ? (p[i].pr <= 1 ? 0 : p[i].pr == 2 ? 1 : 2) : 0;
    while (done < n && t < TMAX) {
        for (int i = 0; i < n; i++) if (!seen[i] && p[i].at <= t) { seen[i] = 1; if (queued) push(p[i].lvl, i); }
        if (queued) {
            if (cur >= 0) for (int k = 0; k < p[cur].lvl; k++) if (qn[k]) { front(p[cur].lvl, cur); cur = -1; break; }
            if (cur < 0) for (int k = 0; k < 3; k++) if (qn[k]) { cur = pop(k); used = 0; break; }
        } else if (cur < 0 || pre) {
            int b = -1;
            for (int i = 0; i < n; i++) {
                if (p[i].at > t || p[i].rem <= 0) continue;
                if (b < 0) { b = i; continue; }
                int ki, kb;
                if (!strcmp(a,"SJF") || !strcmp(a,"SRTF")) { ki = p[i].rem; kb = p[b].rem; }
                else if (!strncmp(a,"PRIORITY",8)) { ki = p[i].pr; kb = p[b].pr; }
                else { ki = p[i].at; kb = p[b].at; }
                if (ki < kb) b = i;
            }
            cur = b;
        }
        if (cur < 0) { seg(-1, t, t + 1); t++; continue; }
        if (p[cur].st < 0) p[cur].st = t;
        seg(p[cur].pid, t, t + 1); p[cur].rem--; t++; used++;
        if (queued) for (int i = 0; i < n; i++) if (!seen[i] && p[i].at <= t) { seen[i] = 1; push(p[i].lvl, i); }
        if (p[cur].rem == 0) { p[cur].ct = t; done++; cur = -1; }
        else if (queued && used >= qt[p[cur].lvl]) { if (mlfq && p[cur].lvl < 2) p[cur].lvl++; push(p[cur].lvl, cur); cur = -1; }
    }
    if (done < n) return fail("simulation limit reached");
    printf("{\"algorithm\":\"%s\",\"gantt\":[", a);
    int busy = 0;
    for (int i = 0; i < gn; i++) {
        if (gp[i] >= 0) busy += ge[i] - gs[i];
        printf("%s{\"pid\":%d,\"idle\":%s,\"start\":%d,\"end\":%d}", i ? "," : "", gp[i] < 0 ? 0 : gp[i], gp[i] < 0 ? "true" : "false", gs[i], ge[i]);
    }
    double tw = 0, tt = 0, tr = 0;
    printf("],\"processes\":[");
    for (int i = 0; i < n; i++) {
        int ta = p[i].ct - p[i].at, w = ta - p[i].bt, r = p[i].st - p[i].at; tw += w; tt += ta; tr += r;
        printf("%s{\"pid\":%d,\"completion\":%d,\"turnaround\":%d,\"waiting\":%d,\"response\":%d}", i ? "," : "", p[i].pid, p[i].ct, ta, w, r);
    }
    printf("],\"avg_waiting\":%.2f,\"avg_turnaround\":%.2f,\"avg_response\":%.2f,\"cpu_utilization\":%.1f,\"throughput\":%.3f}\n", tw/n, tt/n, tr/n, busy * 100.0 / t, (double)n / t);
    return 0;
}
