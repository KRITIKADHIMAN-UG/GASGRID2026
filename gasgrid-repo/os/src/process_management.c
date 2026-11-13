/* GasGrid OS engine: process management (PCB + state machine).
   stdin : N, then N lines "CMD pid burst priority resource"
   CMD   : CREATE ADMIT DISPATCH BLOCK WAKE PREEMPT SUSPEND RESUME TERMINATE   stdout: JSON log + final PCB table */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#define M 64
enum { NEW, READY, RUNNING, WAITING, SUSPENDED, TERMINATED };
static const char *SN[] = {"NEW","READY","RUNNING","WAITING","SUSPENDED","TERMINATED"};
typedef struct { int pid, burst, pri, st; char res[16]; } PCB;
static PCB pc[M]; static int np;
static int find(int pid) { for (int i = 0; i < np; i++) if (pc[i].pid == pid) return i; return -1; }
static int trans(const char *c, int s, int *to) {
    if (!strcmp(c,"ADMIT") && s == NEW) { *to = READY; return 1; }
    if (!strcmp(c,"DISPATCH") && s == READY) { *to = RUNNING; return 1; }
    if (!strcmp(c,"BLOCK") && s == RUNNING) { *to = WAITING; return 1; }
    if (!strcmp(c,"WAKE") && s == WAITING) { *to = READY; return 1; }
    if (!strcmp(c,"PREEMPT") && s == RUNNING) { *to = READY; return 1; }
    if (!strcmp(c,"SUSPEND") && (s == READY || s == WAITING)) { *to = SUSPENDED; return 1; }
    if (!strcmp(c,"RESUME") && s == SUSPENDED) { *to = READY; return 1; }
    if (!strcmp(c,"TERMINATE") && s != TERMINATED) { *to = TERMINATED; return 1; }
    return 0;
}
int main(void) {
    int n;
    if (scanf("%d", &n) != 1 || n < 1 || n > 200) { puts("{\"error\":\"bad input\"}"); return 1; }
    printf("{\"log\":[");
    for (int s = 0; s < n; s++) {
        char c[16], r[16]; int pid, b, pr;
        if (scanf("%15s %d %d %d %15s", c, &pid, &b, &pr, r) != 5) { puts("],\"error\":\"bad command\"}"); return 1; }
        int ok = 1, from = -1, to = -1, i = find(pid); char msg[80] = "ok";
        const char *known[] = {"CREATE","ADMIT","DISPATCH","BLOCK","WAKE","PREEMPT","SUSPEND","RESUME","TERMINATE"}; int kn = 0;
        for (int k = 0; k < 9; k++) if (!strcmp(c, known[k])) kn = 1;
        if (!kn) { ok = 0; strcpy(c, "?"); strcpy(msg, "unknown command"); }
        else if (!strcmp(c, "CREATE")) {
            if (i >= 0) { ok = 0; strcpy(msg, "pid already exists"); }
            else if (np >= M || b < 1) { ok = 0; strcpy(msg, "invalid process"); }
            else { pc[np].pid = pid; pc[np].burst = b; pc[np].pri = pr; pc[np].st = NEW;
                   for (int k = 0, j = 0; r[k] && j < 15; k++) if (isalnum((unsigned char)r[k])) pc[np].res[j++] = r[k];
                   to = NEW; np++; strcpy(msg, "PCB allocated"); }
        } else if (i < 0) { ok = 0; strcpy(msg, "no such process"); }
        else {
            from = pc[i].st;
            if (!trans(c, from, &to)) { ok = 0; snprintf(msg, sizeof msg, "illegal from %s", SN[from]); to = -1; }
            else if (!strcmp(c, "DISPATCH")) { for (int k = 0; k < np; k++) if (pc[k].st == RUNNING) { ok = 0; strcpy(msg, "CPU busy"); to = -1; } }
            if (ok) {
                pc[i].st = to;
                if (!strcmp(c, "BLOCK")) snprintf(msg, sizeof msg, "waiting for %s", pc[i].res);
                if (!strcmp(c, "TERMINATE")) snprintf(msg, sizeof msg, "released %s allocation", pc[i].res);
            }
        }
        printf("%s{\"step\":%d,\"cmd\":\"%s\",\"pid\":%d,\"ok\":%s,\"from\":\"%s\",\"to\":\"%s\",\"msg\":\"%s\"}", s ? "," : "", s + 1, c, pid, ok ? "true" : "false",
               from < 0 ? "-" : SN[from], to < 0 ? "-" : SN[to], msg);
    }
    printf("],\"pcb\":[");
    int cnt[6] = {0};
    for (int i = 0; i < np; i++) { cnt[pc[i].st]++; printf("%s{\"pid\":%d,\"state\":\"%s\",\"burst\":%d,\"priority\":%d,\"resource\":\"%s\"}", i ? "," : "", pc[i].pid, SN[pc[i].st], pc[i].burst, pc[i].pri, pc[i].res); }
    printf("],\"counts\":{");
    for (int k = 0; k < 6; k++) printf("%s\"%s\":%d", k ? "," : "", SN[k], cnt[k]);
    puts("}}");
    return 0;
}
