/* GasGrid OS Engine: Deadlock Management
 * Modes:
 * bank n m   : Banker's safety check and optional resource request
 *              Input: available[m], maximum[n*m], allocation[n*m],
 *                     request_pid (-1 for no request), request[m]
 * detect n m : Deadlock detection algorithm
 *              Input: available[m], allocation[n*m], request[n*m]
 * rag n e    : Wait-for graph cycle detection
 *              Input: e directed edges "a b" (process a waits for b)
 *
 * Output: JSON
 */

#include <stdio.h>
#include <string.h>

#define N 16
#define R 8

static int n, m;
static int av[R], mx[N][R], al[N][R], rq[N][R];

/* Print a standard input-validation error. */
static int bad(void)
{
    puts("{\"error\":\"bad input\"}");
    return 1;
}

/* Check whether all processes can finish in a safe execution order. */
static int safety(const int *w0, int a[N][R], int *seq)
{
    int w[R], fin[N] = {0}, c = 0, pg = 1;

    for (int j = 0; j < R; j++)
        w[j] = w0[j];

    while (pg) {
        pg = 0;

        for (int i = 0; i < n; i++) {
            if (fin[i])
                continue;

            int ok = 1;
            for (int j = 0; j < m; j++) {
                if (mx[i][j] - a[i][j] > w[j]) {
                    ok = 0;
                    break;
                }
            }

            if (ok) {
                for (int j = 0; j < m; j++)
                    w[j] += a[i][j];

                fin[i] = 1;
                seq[c++] = i;
                pg = 1;
            }
        }
    }

    return c == n;
}

/* Print a process sequence as a JSON array. */
static void seqout(const int *s, int c)
{
    printf("[");
    for (int i = 0; i < c; i++)
        printf("%s%d", i ? "," : "", s[i]);
    printf("]");
}

/* Wait-for graph and DFS state used by the RAG mode. */
static int adj[N][N], col[N], stk[N], sp, cyc[N], cn;

/* Detect a directed cycle and store the cycle's process sequence. */
static int dfs(int u)
{
    col[u] = 1;
    stk[sp++] = u;

    for (int v = 0; v < n; v++) {
        if (adj[u][v]) {
            if (col[v] == 1) {
                int k = sp - 1;
                while (stk[k] != v)
                    k--;

                for (; k < sp; k++)
                    cyc[cn++] = stk[k];

                return 1;
            }

            if (col[v] == 0 && dfs(v))
                return 1;
        }
    }

    col[u] = 2;
    sp--;
    return 0;
}

int main(void)
{
    char mode[8], buf[1];
    (void)buf;

    if (scanf("%7s", mode) != 1)
        return bad();

    /* Resource-allocation graph: each edge a -> b means a waits for b. */
    if (!strcmp(mode, "rag")) {
        int e;

        if (scanf("%d %d", &n, &e) != 2 || n < 1 || n > N || e < 0 || e > 100)
            return bad();

        for (int i = 0; i < e; i++) {
            int a, b;
            if (scanf("%d %d", &a, &b) != 2 ||
                a < 0 || b < 0 || a >= n || b >= n)
                return bad();

            adj[a][b] = 1;
        }

        int found = 0;
        for (int i = 0; i < n && !found; i++) {
            if (!col[i])
                found = dfs(i);
        }

        printf("{\"sim\":\"rag\",\"deadlock\":%s,\"cycle\":",
               found ? "true" : "false");
        seqout(cyc, cn);
        puts("}");
        return 0;
    }

    if (scanf("%d %d", &n, &m) != 2 || n < 1 || n > N || m < 1 || m > R)
        return bad();

    /* Read the currently available instances of each resource type. */
    for (int j = 0; j < m; j++) {
        if (scanf("%d", &av[j]) != 1 || av[j] < 0)
            return bad();
    }

    if (!strcmp(mode, "bank")) {
        int seq[N], rp, rv[R];

        /* Read maximum demand and current allocation matrices. */
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                if (scanf("%d", &mx[i][j]) != 1 || mx[i][j] < 0)
                    return bad();
            }
        }

        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                if (scanf("%d", &al[i][j]) != 1 ||
                    al[i][j] < 0 || al[i][j] > mx[i][j])
                    return bad();
            }
        }

        /* Read the optional requesting process and its resource vector. */
        if (scanf("%d", &rp) != 1 || rp < -1 || rp >= n)
            return bad();

        for (int j = 0; j < m; j++) {
            if (scanf("%d", &rv[j]) != 1 || rv[j] < 0)
                return bad();
        }

        int safe = safety(av, al, seq), c = 0;
        for (int i = 0; i < n; i++)
            c += 0;

        /* Report each process's remaining resource need and safety result. */
        printf("{\"sim\":\"bank\",\"need\":[");
        for (int i = 0; i < n; i++) {
            printf("%s[", i ? "," : "");
            for (int j = 0; j < m; j++)
                printf("%s%d", j ? "," : "", mx[i][j] - al[i][j]);
            printf("]");
        }

        printf("],\"safe\":%s,\"sequence\":", safe ? "true" : "false");
        if (safe)
            seqout(seq, n);
        else
            printf("[]");

        /* Validate the request and grant it only if the resulting state is safe. */
        if (rp >= 0) {
            const char *d = "GRANTED";
            int s2[N], a2[N][R], w2[R];

            for (int j = 0; j < m; j++) {
                if (rv[j] > mx[rp][j] - al[rp][j])
                    d = "DENIED_EXCEEDS_MAX";
            }

            if (!strcmp(d, "GRANTED")) {
                for (int j = 0; j < m; j++) {
                    if (rv[j] > av[j])
                        d = "MUST_WAIT";
                }
            }

            if (!strcmp(d, "GRANTED")) {
                memcpy(a2, al, sizeof a2);

                for (int j = 0; j < m; j++) {
                    a2[rp][j] += rv[j];
                    w2[j] = av[j] - rv[j];
                }

                if (!safety(w2, a2, s2)) {
                    d = "DENIED_UNSAFE";
                } else {
                    printf(",\"request\":{\"pid\":%d,\"decision\":\"%s\",\"sequence\":",
                           rp, d);
                    seqout(s2, n);
                    printf("}");
                    d = 0;
                }
            }

            if (d)
                printf(",\"request\":{\"pid\":%d,\"decision\":\"%s\"}", rp, d);
        }

        puts("}");
    } else if (!strcmp(mode, "detect")) {
        int fin[N] = {0}, w[R], pg = 1;

        /* Read allocation and outstanding-request matrices. */
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                if (scanf("%d", &al[i][j]) != 1 || al[i][j] < 0)
                    return bad();
            }
        }

        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                if (scanf("%d", &rq[i][j]) != 1 || rq[i][j] < 0)
                    return bad();
            }
        }

        /* Initialize the work vector and mark processes holding no resources. */
        for (int j = 0; j < m; j++)
            w[j] = av[j];

        for (int i = 0; i < n; i++) {
            int z = 1;
            for (int j = 0; j < m; j++) {
                if (al[i][j])
                    z = 0;
            }
            fin[i] = z;
        }

        /* Repeatedly release resources from processes whose requests can be met. */
        while (pg) {
            pg = 0;

            for (int i = 0; i < n; i++) {
                if (fin[i])
                    continue;

                int ok = 1;
                for (int j = 0; j < m; j++) {
                    if (rq[i][j] > w[j])
                        ok = 0;
                }

                if (ok) {
                    for (int j = 0; j < m; j++)
                        w[j] += al[i][j];

                    fin[i] = 1;
                    pg = 1;
                }
            }
        }

        /* Processes that cannot finish are reported as deadlocked. */
        int d[N], dc = 0;
        for (int i = 0; i < n; i++) {
            if (!fin[i])
                d[dc++] = i;
        }

        printf("{\"sim\":\"detect\",\"deadlock\":%s,\"deadlocked\":",
               dc ? "true" : "false");
        seqout(d, dc);
        puts("}");
    } else {
        return bad();
    }

    return 0;
}
