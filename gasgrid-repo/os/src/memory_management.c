/* GasGrid OS engine: memory management.
   "fit FIRST|BEST|WORST nh holes.. nj jobs.."
   "page FIFO|LRU|OPT|CLOCK|LFU|MFU frames n refs.."
   "table pagesize np frame_of_page.. (-1 = not loaded) na logical_addresses.." */
#include <stdio.h>
#include <string.h>
#define MP 1000
static int bad(void) { puts("{\"error\":\"bad input\"}"); return 1; }
static int run(const char *a, const int *ref, int n, int F, int pr) {
    int fr[16], cnt[MP] = {0}, ld[MP] = {0}, lt[MP] = {0}, rb[16] = {0}, nf = 0, ptr = 0, faults = 0;
    for (int i = 0; i < n; i++) {
        int pg = ref[i], hit = -1;
        for (int j = 0; j < nf; j++) if (fr[j] == pg) hit = j;
        if (hit >= 0) { cnt[pg]++; ld[pg] = i; if (!strcmp(a, "CLOCK")) rb[hit] = 1; }
        else {
            faults++; int idx = 0;
            if (nf < F) { idx = nf++; ptr = (idx + 1) % F; }
            else if (!strcmp(a, "FIFO")) { idx = ptr; ptr = (ptr + 1) % F; }
            else if (!strcmp(a, "CLOCK")) { while (rb[ptr]) { rb[ptr] = 0; ptr = (ptr + 1) % F; } idx = ptr; ptr = (ptr + 1) % F; }
            else {
                long best = -2000000000L;
                for (int j = 0; j < F; j++) {
                    int p = fr[j]; long k;
                    if (!strcmp(a, "LRU")) k = -ld[p];
                    else if (!strcmp(a, "OPT")) { int nx = -1; for (int x = i + 1; x < n; x++) if (ref[x] == p) { nx = x; break; } k = nx < 0 ? 1000000 : nx; }
                    else if (!strcmp(a, "LFU")) k = -(long)cnt[p] * 100000 - lt[p];
                    else k = (long)cnt[p] * 100000 - lt[p];
                    if (k > best) { best = k; idx = j; }
                }
            }
            fr[idx] = pg; cnt[pg] = 1; ld[pg] = i; lt[pg] = i; rb[idx] = 1;
        }
        if (pr) { printf("%s{\"page\":%d,\"fault\":%s,\"frames\":[", i ? "," : "", pg, hit < 0 ? "true" : "false"); for (int j = 0; j < F; j++) printf("%s%d", j ? "," : "", j < nf ? fr[j] : -1); printf("]}"); }
    }
    return faults;
}
int main(void) {
    char mode[8], a[8];
    if (scanf("%7s", mode) != 1) return bad();
    if (!strcmp(mode, "fit")) {
        int nh, nj, h[32], j[32], used[32];
        if (scanf("%7s %d", a, &nh) != 2 || nh < 1 || nh > 32) return bad();
        for (int i = 0; i < nh; i++) if (scanf("%d", &h[i]) != 1 || h[i] < 1) return bad();
        if (scanf("%d", &nj) != 1 || nj < 1 || nj > 32) return bad();
        for (int i = 0; i < nj; i++) if (scanf("%d", &j[i]) != 1 || j[i] < 1) return bad();
        if (strcmp(a, "FIRST") && strcmp(a, "BEST") && strcmp(a, "WORST")) return bad();
        int orig[32]; memcpy(orig, h, sizeof(int) * nh); memset(used, 0, sizeof used);
        printf("{\"sim\":\"fit\",\"algorithm\":\"%s\",\"allocations\":[", a);
        for (int i = 0; i < nj; i++) {
            int b = -1;
            for (int k = 0; k < nh; k++) {
                if (h[k] < j[i]) continue;
                if (b < 0 || (!strcmp(a, "BEST") && h[k] < h[b]) || (!strcmp(a, "WORST") && h[k] > h[b])) b = k;
                if (!strcmp(a, "FIRST")) break;
            }
            if (b >= 0) { h[b] -= j[i]; used[b] += j[i]; }
            printf("%s{\"job\":%d,\"size\":%d,\"hole\":%d,\"left_in_hole\":%d}", i ? "," : "", i, j[i], b, b < 0 ? 0 : h[b]);
        }
        int tot = 0, big = 0; for (int k = 0; k < nh; k++) { tot += h[k]; if (h[k] > big) big = h[k]; }
        printf("],\"holes\":["); for (int k = 0; k < nh; k++) printf("%s%d", k ? "," : "", h[k]);
        printf("],\"total_free\":%d,\"largest_hole\":%d}\n", tot, big);
    } else if (!strcmp(mode, "page")) {
        int F, n, ref[200];
        if (scanf("%7s %d %d", a, &F, &n) != 3 || F < 1 || F > 8 || n < 1 || n > 200) return bad();
        for (int i = 0; i < n; i++) if (scanf("%d", &ref[i]) != 1 || ref[i] < 0 || ref[i] >= MP) return bad();
        const char *al[] = {"FIFO","LRU","OPT","CLOCK","LFU","MFU"}; int ok = 0; for (int i = 0; i < 6; i++) if (!strcmp(a, al[i])) ok = 1;
        if (!ok) return bad();
        printf("{\"sim\":\"page\",\"algorithm\":\"%s\",\"frames\":%d,\"steps\":[", a, F);
        int f = run(a, ref, n, F, 1);
        printf("],\"faults\":%d,\"fault_rate\":%.1f,\"comparison\":{", f, f * 100.0 / n);
        for (int i = 0; i < 6; i++) printf("%s\"%s\":%d", i ? "," : "", al[i], run(al[i], ref, n, F, 0));
        int f1 = run("FIFO", ref, n, F, 0), f2 = run("FIFO", ref, n, F + 1, 0);
        printf("},\"belady\":{\"fifo_faults\":[%d,%d],\"anomaly\":%s}}\n", f1, f2, f2 > f1 ? "true" : "false");
    } else if (!strcmp(mode, "table")) {
        int ps, np, na, pt[64];
        if (scanf("%d %d", &ps, &np) != 2 || ps < 1 || np < 1 || np > 64) return bad();
        for (int i = 0; i < np; i++) if (scanf("%d", &pt[i]) != 1 || pt[i] < -1) return bad();
        if (scanf("%d", &na) != 1 || na < 1 || na > 64) return bad();
        printf("{\"sim\":\"table\",\"translations\":[");
        for (int i = 0; i < na; i++) {
            int la, pg, off; if (scanf("%d", &la) != 1 || la < 0) return bad();
            pg = la / ps; off = la % ps;
            if (pg >= np || pt[pg] < 0) printf("%s{\"logical\":%d,\"page\":%d,\"offset\":%d,\"status\":\"PAGE_FAULT\"}", i ? "," : "", la, pg, off);
            else printf("%s{\"logical\":%d,\"page\":%d,\"offset\":%d,\"frame\":%d,\"physical\":%d,\"status\":\"OK\"}", i ? "," : "", la, pg, off, pt[pg], pt[pg] * ps + off);
        }
        puts("]}");
    } else return bad();
    return 0;
}
