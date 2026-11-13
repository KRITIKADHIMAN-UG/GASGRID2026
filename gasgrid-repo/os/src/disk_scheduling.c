/* GasGrid OS engine: disk scheduling.
   stdin: ALGO head max direction(up|down) n requests..   ALGO: FCFS SSTF SCAN CSCAN LOOK CLOOK */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define M 64
static int cmp(const void *a, const void *b) { return *(const int *)a - *(const int *)b; }
static int path(const char *a, const int *q, int n, int h, int mx, int up, int *p) {
    int u[M], l[M], nu = 0, nl = 0, c = 0;
    for (int i = 0; i < n; i++) { if (q[i] >= h) u[nu++] = q[i]; else l[nl++] = q[i]; }
    qsort(u, nu, sizeof(int), cmp); qsort(l, nl, sizeof(int), cmp);       /* both ascending */
    p[c++] = h;
    if (!strcmp(a, "FCFS")) for (int i = 0; i < n; i++) p[c++] = q[i];
    else if (!strcmp(a, "SSTF")) {
        int r[M], k = n, cur = h; memcpy(r, q, sizeof(int) * n);
        while (k) { int b = 0; for (int i = 1; i < k; i++) if (abs(r[i] - cur) < abs(r[b] - cur)) b = i; cur = r[b]; p[c++] = cur; r[b] = r[--k]; }
    } else {
        int scan = !strcmp(a, "SCAN"), cs = !strcmp(a, "CSCAN"), look = !strcmp(a, "LOOK");
        if (up) {
            for (int i = 0; i < nu; i++) p[c++] = u[i];
            if (scan) { p[c++] = mx; for (int i = nl - 1; i >= 0; i--) p[c++] = l[i]; }
            else if (cs) { p[c++] = mx; p[c++] = 0; for (int i = 0; i < nl; i++) p[c++] = l[i]; }
            else if (look) { for (int i = nl - 1; i >= 0; i--) p[c++] = l[i]; }
            else { for (int i = 0; i < nl; i++) p[c++] = l[i]; }
        } else {
            for (int i = nl - 1; i >= 0; i--) p[c++] = l[i];
            if (scan) { p[c++] = 0; for (int i = 0; i < nu; i++) p[c++] = u[i]; }
            else if (cs) { p[c++] = 0; p[c++] = mx; for (int i = nu - 1; i >= 0; i--) p[c++] = u[i]; }
            else if (look) { for (int i = 0; i < nu; i++) p[c++] = u[i]; }
            else { for (int i = nu - 1; i >= 0; i--) p[c++] = u[i]; }
        }
    }
    return c;
}
static int mv(const int *p, int c) { int s = 0; for (int i = 1; i < c; i++) s += abs(p[i] - p[i-1]); return s; }
int main(void) {
    char a[8], d[8]; int h, mx, n, q[M], p[3 * M];
    if (scanf("%7s %d %d %7s %d", a, &h, &mx, d, &n) != 5 || n < 1 || n > M || mx < 1 || mx > 100000 || h < 0 || h > mx || (strcmp(d, "up") && strcmp(d, "down"))) { puts("{\"error\":\"bad input\"}"); return 1; }
    for (int i = 0; i < n; i++) if (scanf("%d", &q[i]) != 1 || q[i] < 0 || q[i] > mx) { puts("{\"error\":\"bad request\"}"); return 1; }
    const char *al[] = {"FCFS","SSTF","SCAN","CSCAN","LOOK","CLOOK"}; int ok = 0; for (int i = 0; i < 6; i++) if (!strcmp(a, al[i])) ok = 1;
    if (!ok) { puts("{\"error\":\"unknown algorithm\"}"); return 1; }
    int up = !strcmp(d, "up"), c = path(a, q, n, h, mx, up, p);
    printf("{\"sim\":\"disk\",\"algorithm\":\"%s\",\"path\":[", a);
    for (int i = 0; i < c; i++) printf("%s%d", i ? "," : "", p[i]);
    printf("],\"total_movement\":%d,\"comparison\":{", mv(p, c));
    for (int i = 0; i < 6; i++) { int pc = path(al[i], q, n, h, mx, up, p); printf("%s\"%s\":%d", i ? "," : "", al[i], mv(p, pc)); }
    puts("}}");
    return 0;
}
