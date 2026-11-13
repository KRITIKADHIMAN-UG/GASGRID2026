/* GasGrid OS engine: synchronization simulations (deterministic interleavings).
   stdin: "race stock safe" | "sem init ops(P/V)" | "pc cap ops(P/C)" | "peterson iters" | "tas|cas|none iters"
          | "rw seq(R/W)" | "dine n NAIVE|ORDERED rounds" */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#define TM 400
static char T[TM][110]; static int tn;
static void tr(const char *f, ...) { if (tn >= TM) return; va_list a; va_start(a, f); vsnprintf(T[tn++], 110, f, a); va_end(a); }
static void out(const char *sim, const char *res) {
    printf("{\"sim\":\"%s\",\"trace\":[", sim);
    for (int i = 0; i < tn; i++) printf("%s\"%s\"", i ? "," : "", T[i]);
    printf("],\"result\":%s}\n", res);
}
static int bad(void) { puts("{\"error\":\"bad input\"}"); return 1; }
static int imin(int a, int b) { return a < b ? a : b; }
static int imax(int a, int b) { return a > b ? a : b; }
int main(void) {
    char sim[16], ops[128], r[300];
    if (scanf("%15s", sim) != 1) return bad();
    if (!strcmp(sim, "race")) {
        int s, safe, sold, fin;
        if (scanf("%d %d", &s, &safe) != 2 || s < 1 || s > 1000) return bad();
        if (!safe) {
            tr("T1 reads stock=%d", s); tr("T2 reads stock=%d", s);
            tr("T1 writes stock=%d and confirms booking", s - 1); tr("T2 writes stock=%d and confirms booking", s - 1); sold = 2; fin = s - 1;
        } else {
            tr("T1 acquires mutex"); tr("T1 reads %d, writes %d, confirms booking", s, s - 1); tr("T1 releases mutex"); tr("T2 acquires mutex");
            if (s >= 2) { tr("T2 reads %d, writes %d, confirms booking", s - 1, s - 2); sold = 2; fin = s - 2; }
            else { tr("T2 reads 0 and rejects booking: out of stock"); sold = 1; fin = 0; }
            tr("T2 releases mutex");
        }
        snprintf(r, sizeof r, "{\"initial_stock\":%d,\"sold\":%d,\"final_stock\":%d,\"oversold\":%s,\"lost_update\":%s}", s, sold, fin, sold > s ? "true" : "false", fin != s - sold ? "true" : "false");
        out(sim, r);
    } else if (!strcmp(sim, "sem")) {
        int v, w = 0, id = 0;
        if (scanf("%d %100s", &v, ops) != 2 || v < 0 || v > 100) return bad();
        for (int k = 0; ops[k]; k++) {
            if (ops[k] == 'P') { id++; if (v > 0) { v--; tr("P by B%d acquired a unit (value=%d)", id, v); } else { w++; tr("P by B%d BLOCKED (queue=%d)", id, w); } }
            else if (ops[k] == 'V') { if (w > 0) { w--; tr("V wakes a blocked booking (queue=%d)", w); } else { v++; tr("V releases a unit (value=%d)", v); } }
            else return bad();
        }
        snprintf(r, sizeof r, "{\"value\":%d,\"blocked\":%d}", v, w); out(sim, r);
    } else if (!strcmp(sim, "pc")) {
        int cap, c = 0, pid = 0, cid = 0;
        if (scanf("%d %100s", &cap, ops) != 2 || cap < 1 || cap > 50) return bad();
        for (int k = 0; ops[k]; k++) {
            if (ops[k] == 'P') { if (c == cap) tr("Producer BLOCKED: buffer full (%d/%d)", c, cap); else { c++; pid++; tr("Produced R%d (%d/%d)", pid, c, cap); } }
            else if (ops[k] == 'C') { if (c == 0) tr("Consumer BLOCKED: buffer empty"); else { c--; cid++; tr("Dispatcher consumed R%d (%d/%d)", cid, c, cap); } }
            else return bad();
        }
        snprintf(r, sizeof r, "{\"buffered\":%d,\"produced\":%d,\"consumed\":%d,\"full\":%s,\"empty\":%s}", c, pid, cid, c == cap ? "true" : "false", c == 0 ? "true" : "false"); out(sim, r);
    } else if (!strcmp(sim, "peterson")) {
        int N, flag[2] = {0}, turn = 0, st[2] = {0}, it[2] = {0}, tmp[2] = {0}, cnt = 0, in = 0, viol = 0, steps = 0, ent = 0;
        if (scanf("%d", &N) != 1 || N < 1 || N > 1000) return bad();
        while ((it[0] < N || it[1] < N) && steps < 10000000) {
            for (int i = 0; i < 2; i++) {
                if (it[i] >= N) continue; int o = 1 - i; steps++;
                switch (st[i]) {
                    case 0: flag[i] = 1; st[i] = 1; break;
                    case 1: turn = o; st[i] = 2; break;
                    case 2: if (flag[o] && turn == o) break; st[i] = 3; in++; if (in > 1) viol++; if (ent++ < 12) tr("step %d: P%d enters critical section", steps, i); break;
                    case 3: tmp[i] = cnt; st[i] = 4; break;
                    case 4: cnt = tmp[i] + 1; st[i] = 5; break;
                    case 5: flag[i] = 0; in--; it[i]++; st[i] = 0; break;
                }
            }
        }
        snprintf(r, sizeof r, "{\"expected\":%d,\"counter\":%d,\"violations\":%d,\"correct\":%s}", 2 * N, cnt, viol, cnt == 2 * N && !viol ? "true" : "false"); out(sim, r);
    } else if (!strcmp(sim, "tas") || !strcmp(sim, "cas") || !strcmp(sim, "none")) {
        int N, lock = 0, st[2] = {0}, it[2] = {0}, tmp[2] = {0}, cnt = 0, in = 0, viol = 0, steps = 0, ent = 0, none = !strcmp(sim, "none");
        if (scanf("%d", &N) != 1 || N < 1 || N > 1000) return bad();
        while ((it[0] < N || it[1] < N) && steps < 10000000) {
            for (int i = 0; i < 2; i++) {
                if (it[i] >= N) continue; steps++;
                switch (st[i]) {
                    case 0: if (none || lock == 0) { lock = 1; st[i] = 1; in++; if (in > 1) viol++; if (ent++ < 12) tr("P%d %s", i, none ? "enters with NO protection" : !strcmp(sim, "tas") ? "TestAndSet(lock) returned 0: enters" : "CompareAndSwap(lock,0,1) succeeded: enters"); } break;
                    case 1: tmp[i] = cnt; st[i] = 2; break;
                    case 2: cnt = tmp[i] + 1; st[i] = 3; break;
                    case 3: lock = 0; in--; it[i]++; st[i] = 0; break;
                }
            }
        }
        snprintf(r, sizeof r, "{\"expected\":%d,\"counter\":%d,\"violations\":%d,\"correct\":%s}", 2 * N, cnt, viol, cnt == 2 * N && !viol ? "true" : "false"); out(sim, r);
    } else if (!strcmp(sim, "rw")) {
        int rd = 0, wr = 0, k = 0;
        if (scanf("%100s", ops) != 1) return bad();
        while (ops[k]) {
            if (ops[k] == 'R') { int c = 0; while (ops[k] == 'R') { c++; k++; rd++; } tr("%d reader(s) read stock concurrently", c); }
            else if (ops[k] == 'W') { wr++; k++; tr("Writer W%d has exclusive access; readers wait", wr); }
            else return bad();
        }
        snprintf(r, sizeof r, "{\"readers\":%d,\"writers\":%d}", rd, wr); out(sim, r);
    } else if (!strcmp(sim, "dine")) {
        int n, rounds, meals = 0, dead = 0; char st[16];
        if (scanf("%d %15s %d", &n, st, &rounds) != 3 || n < 2 || n > 10 || rounds < 1 || rounds > 20) return bad();
        int owner[10], held[10];
        if (!strcmp(st, "NAIVE")) {
            for (int i = 0; i < n; i++) tr("Philosopher %d takes left fork %d", i, i);
            for (int i = 0; i < n; i++) tr("Philosopher %d waits for fork %d held by %d", i, (i + 1) % n, (i + 1) % n);
            tr("DEADLOCK: circular wait among all %d philosophers", n); dead = 1;
        } else if (!strcmp(st, "ORDERED")) {
            for (int rd = 0; rd < rounds; rd++) {
                for (int i = 0; i < n; i++) { owner[i] = -1; held[i] = 0; }
                int prog = 1;
                while (prog) {
                    prog = 0;
                    for (int i = 0; i < n; i++) {
                        int f = imin(i, (i + 1) % n), s = imax(i, (i + 1) % n);
                        if (held[i] == 0 && owner[f] < 0) { owner[f] = i; held[i] = 1; prog = 1; }
                        else if (held[i] == 1 && owner[s] < 0) { meals++; tr("Round %d: philosopher %d eats (forks %d,%d)", rd + 1, i, f, s); owner[f] = -1; held[i] = 2; prog = 1; }
                    }
                }
            }
            tr("No deadlock: forks are always taken in ascending order");
        } else return bad();
        snprintf(r, sizeof r, "{\"deadlock\":%s,\"meals\":%d}", dead ? "true" : "false", meals); out(sim, r);
    } else return bad();
    return 0;
}
