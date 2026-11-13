/* GasGrid OS engine: deadlocks.
   "bank n m" avail[m] max[n*m] alloc[n*m] reqpid(-1 none) req[m]   -> Banker's safety + resource request
   "detect n m" avail[m] alloc[n*m] request[n*m]                     -> detection algorithm
   "rag n e" then e pairs "a b" (a waits for b)                       -> wait-for graph cycle detection */
#include <stdio.h>
#include <string.h>
#define N 16
#define R 8
static int n, m, av[R], mx[N][R], al[N][R], rq[N][R];
static int bad(void) { puts("{\"error\":\"bad input\"}"); return 1; }
static int safety(const int *w0, int a[N][R], int *seq) {
    int w[R], fin[N] = {0}, c = 0, pg = 1;
    for (int j = 0; j < R; j++) w[j] = w0[j];
    while (pg) {
        pg = 0;
        for (int i = 0; i < n; i++) {
            if (fin[i]) continue; int ok = 1;
            for (int j = 0; j < m; j++) if (mx[i][j] - a[i][j] > w[j]) { ok = 0; break; }
            if (ok) { for (int j = 0; j < m; j++) w[j] += a[i][j]; fin[i] = 1; seq[c++] = i; pg = 1; }
        }
    }
    return c == n;
}
static void seqout(const int *s, int c) { printf("["); for (int i = 0; i < c; i++) printf("%s%d", i ? "," : "", s[i]); printf("]"); }
static int adj[N][N], col[N], stk[N], sp, cyc[N], cn;
static int dfs(int u) {
    col[u] = 1; stk[sp++] = u;
    for (int v = 0; v < n; v++) if (adj[u][v]) {
        if (col[v] == 1) { int k = sp - 1; while (stk[k] != v) k--; for (; k < sp; k++) cyc[cn++] = stk[k]; return 1; }
        if (col[v] == 0 && dfs(v)) return 1;
    }
    col[u] = 2; sp--; return 0;
}
int main(void) {
    char mode[8], buf[1];
    (void)buf;
    if (scanf("%7s", mode) != 1) return bad();
    if (!strcmp(mode, "rag")) {
        int e;
        if (scanf("%d %d", &n, &e) != 2 || n < 1 || n > N || e < 0 || e > 100) return bad();
        for (int i = 0; i < e; i++) { int a, b; if (scanf("%d %d", &a, &b) != 2 || a < 0 || b < 0 || a >= n || b >= n) return bad(); adj[a][b] = 1; }
        int found = 0; for (int i = 0; i < n && !found; i++) if (!col[i]) found = dfs(i);
        printf("{\"sim\":\"rag\",\"deadlock\":%s,\"cycle\":", found ? "true" : "false"); seqout(cyc, cn); puts("}");
        return 0;
    }
    if (scanf("%d %d", &n, &m) != 2 || n < 1 || n > N || m < 1 || m > R) return bad();
    for (int j = 0; j < m; j++) if (scanf("%d", &av[j]) != 1 || av[j] < 0) return bad();
    if (!strcmp(mode, "bank")) {
        int seq[N], rp, rv[R];
        for (int i = 0; i < n; i++) for (int j = 0; j < m; j++) if (scanf("%d", &mx[i][j]) != 1 || mx[i][j] < 0) return bad();
        for (int i = 0; i < n; i++) for (int j = 0; j < m; j++) if (scanf("%d", &al[i][j]) != 1 || al[i][j] < 0 || al[i][j] > mx[i][j]) return bad();
        if (scanf("%d", &rp) != 1 || rp < -1 || rp >= n) return bad();
        for (int j = 0; j < m; j++) if (scanf("%d", &rv[j]) != 1 || rv[j] < 0) return bad();
        int safe = safety(av, al, seq), c = 0; for (int i = 0; i < n; i++) c += 0;
        printf("{\"sim\":\"bank\",\"need\":[");
        for (int i = 0; i < n; i++) { printf("%s[", i ? "," : ""); for (int j = 0; j < m; j++) printf("%s%d", j ? "," : "", mx[i][j] - al[i][j]); printf("]"); }
        printf("],\"safe\":%s,\"sequence\":", safe ? "true" : "false");
        if (safe) seqout(seq, n); else printf("[]");
        if (rp >= 0) {
            const char *d = "GRANTED"; int s2[N], a2[N][R], w2[R];
            for (int j = 0; j < m; j++) { if (rv[j] > mx[rp][j] - al[rp][j]) d = "DENIED_EXCEEDS_MAX"; }
            if (!strcmp(d, "GRANTED")) for (int j = 0; j < m; j++) if (rv[j] > av[j]) d = "MUST_WAIT";
            if (!strcmp(d, "GRANTED")) {
                memcpy(a2, al, sizeof a2);
                for (int j = 0; j < m; j++) { a2[rp][j] += rv[j]; w2[j] = av[j] - rv[j]; }
                if (!safety(w2, a2, s2)) d = "DENIED_UNSAFE"; else { printf(",\"request\":{\"pid\":%d,\"decision\":\"%s\",\"sequence\":", rp, d); seqout(s2, n); printf("}"); d = 0; }
            }
            if (d) printf(",\"request\":{\"pid\":%d,\"decision\":\"%s\"}", rp, d);
        }
        puts("}");
    } else if (!strcmp(mode, "detect")) {
        int fin[N] = {0}, w[R], pg = 1;
        for (int i = 0; i < n; i++) for (int j = 0; j < m; j++) if (scanf("%d", &al[i][j]) != 1 || al[i][j] < 0) return bad();
        for (int i = 0; i < n; i++) for (int j = 0; j < m; j++) if (scanf("%d", &rq[i][j]) != 1 || rq[i][j] < 0) return bad();
        for (int j = 0; j < m; j++) w[j] = av[j];
        for (int i = 0; i < n; i++) { int z = 1; for (int j = 0; j < m; j++) if (al[i][j]) z = 0; fin[i] = z; }
        while (pg) {
            pg = 0;
            for (int i = 0; i < n; i++) {
                if (fin[i]) continue; int ok = 1;
                for (int j = 0; j < m; j++) if (rq[i][j] > w[j]) ok = 0;
                if (ok) { for (int j = 0; j < m; j++) w[j] += al[i][j]; fin[i] = 1; pg = 1; }
            }
        }
        int d[N], dc = 0; for (int i = 0; i < n; i++) if (!fin[i]) d[dc++] = i;
        printf("{\"sim\":\"detect\",\"deadlock\":%s,\"deadlocked\":", dc ? "true" : "false"); seqout(d, dc); puts("}");
    } else return bad();
    return 0;
}
