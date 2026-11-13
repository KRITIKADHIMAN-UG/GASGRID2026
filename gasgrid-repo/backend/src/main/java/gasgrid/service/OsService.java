package gasgrid.service;

import gasgrid.util.*;
import java.math.BigDecimal;
import java.util.*;

/** Validates JSON from the client, converts it to the C engine's text input, runs it, and records the run. */
public final class OsService {
    private OsService() {}
    private static ApiException bad(String m) { return new ApiException(400, m); }
    private static String word(Object o, String re) { String s = o == null ? "" : (o instanceof BigDecimal ? ((BigDecimal) o).toPlainString() : o.toString()); if (!s.matches(re)) throw bad("Invalid value: " + (s.length() > 20 ? s.substring(0, 20) : s)); return s; }
    private static long num(Object o, long lo, long hi) {
        try { long v = o instanceof BigDecimal ? ((BigDecimal) o).longValueExact() : Long.parseLong(String.valueOf(o).trim()); if (v < lo || v > hi) throw bad("Number out of range"); return v; }
        catch (NumberFormatException e) { throw bad("Number expected"); } catch (ArithmeticException e) { throw bad("Whole number expected"); }
    }
    @SuppressWarnings("unchecked") private static List<Object> list(Object o, int max) { if (!(o instanceof List)) throw bad("List expected"); List<Object> l = (List<Object>) o; if (l.size() > max) throw bad("List too long"); return l; }
    @SuppressWarnings("unchecked") private static Map<String, Object> obj(Object o) { if (!(o instanceof Map)) throw bad("Object expected"); return (Map<String, Object>) o; }
    private static long[] vec(Object o, int max, long lo, long hi) { List<Object> l = list(o, max); if (l.isEmpty()) throw bad("Empty list"); long[] v = new long[l.size()]; for (int i = 0; i < v.length; i++) v[i] = num(l.get(i), lo, hi); return v; }
    private static long[][] matrix(Object o, int maxRows, int cols) {
        List<Object> l = list(o, maxRows); if (l.isEmpty()) throw bad("Empty matrix");
        long[][] m = new long[l.size()][];
        for (int i = 0; i < m.length; i++) { m[i] = vec(l.get(i), 8, 0, 1000000); if (m[i].length != cols) throw bad("Matrix must have " + cols + " columns"); }
        return m;
    }
    private static void join(StringBuilder sb, long[] v) { for (int i = 0; i < v.length; i++) sb.append(i > 0 ? " " : "").append(v[i]); sb.append('\n'); }

    /** Short label stored in the OS_SIMULATIONS table (algorithm or simulation name). */
    public static String label(String module, Map<String, Object> b) {
        Object a = b.get("algo") != null ? b.get("algo") : b.get("sim");
        return a == null ? module : word(a, "[A-Za-z_]{1,20}");
    }
    public static String translate(String module, Map<String, Object> b) {
        StringBuilder sb = new StringBuilder();
        switch (module) {
            case "cpu": {
                List<Object> ps = list(b.get("procs"), 64); if (ps.isEmpty()) throw bad("Add at least one process");
                sb.append(word(b.get("algo"), "[A-Z_]{2,12}")).append(' ').append(b.containsKey("quantum") ? num(b.get("quantum"), 1, 100) : 2).append(' ').append(ps.size()).append('\n');
                for (Object x : ps) { Map<String, Object> p = obj(x);
                    sb.append(num(p.get("pid"), 0, 100000)).append(' ').append(num(p.get("arrival"), 0, 1000)).append(' ').append(num(p.get("burst"), 1, 1000)).append(' ').append(p.containsKey("priority") ? num(p.get("priority"), 0, 100) : 1).append('\n'); }
                break; }
            case "process": {
                List<Object> cs = list(b.get("commands"), 200); if (cs.isEmpty()) throw bad("No commands");
                sb.append(cs.size()).append('\n');
                for (Object x : cs) { Map<String, Object> c = obj(x);
                    sb.append(word(c.get("cmd"), "[A-Z]{3,10}")).append(' ').append(num(c.get("pid"), 0, 100000)).append(' ').append(c.containsKey("burst") ? num(c.get("burst"), 0, 1000) : 1).append(' ')
                      .append(c.containsKey("priority") ? num(c.get("priority"), 0, 100) : 1).append(' ').append(c.containsKey("resource") ? word(c.get("resource"), "[A-Za-z0-9]{1,12}") : "NONE").append('\n'); }
                break; }
            case "sync": {
                sb.append(word(b.get("sim"), "[a-z]{2,10}"));
                for (Object a : list(b.get("args"), 6)) sb.append(' ').append(word(a, "[A-Za-z0-9]{1,100}"));
                sb.append('\n'); break; }
            case "deadlock": {
                String sim = word(b.get("sim"), "[a-z]{3,6}");
                if (sim.equals("rag")) {
                    List<Object> e = list(b.get("edges"), 100);
                    sb.append("rag ").append(num(b.get("n"), 1, 16)).append(' ').append(e.size()).append('\n');
                    for (Object x : e) { List<Object> p = list(x, 2); if (p.size() != 2) throw bad("Edge needs 2 nodes"); sb.append(num(p.get(0), 0, 15)).append(' ').append(num(p.get(1), 0, 15)).append('\n'); }
                } else if (sim.equals("bank") || sim.equals("detect")) {
                    long[] av = vec(b.get("available"), 8, 0, 1000000); int m = av.length; long[][] al = matrix(b.get("alloc"), 16, m); int n = al.length;
                    sb.append(sim).append(' ').append(n).append(' ').append(m).append('\n'); join(sb, av);
                    if (sim.equals("bank")) {
                        long[][] mx = matrix(b.get("max"), 16, m); if (mx.length != n) throw bad("Max and Allocation need the same number of rows");
                        for (long[] r : mx) join(sb, r); for (long[] r : al) join(sb, r);
                        long pid = -1; long[] rv = new long[m];
                        if (b.get("request") instanceof Map) { Map<String, Object> r = obj(b.get("request")); pid = num(r.get("pid"), 0, n - 1); rv = vec(r.get("vector"), 8, 0, 1000000); if (rv.length != m) throw bad("Request vector length"); }
                        sb.append(pid).append(' '); join(sb, rv);
                    } else {
                        long[][] rq = matrix(b.get("request"), 16, m); if (rq.length != n) throw bad("Request matrix rows");
                        for (long[] r : al) join(sb, r); for (long[] r : rq) join(sb, r);
                    }
                } else throw bad("Unknown deadlock simulation");
                break; }
            case "memory": {
                String sim = word(b.get("sim"), "[a-z]{3,5}");
                if (sim.equals("fit")) { long[] h = vec(b.get("holes"), 32, 1, 1000000), j = vec(b.get("jobs"), 32, 1, 1000000);
                    sb.append("fit ").append(word(b.get("algo"), "[A-Z]{3,5}")).append(' ').append(h.length).append(' '); join(sb, h); sb.append(j.length).append(' '); join(sb, j); }
                else if (sim.equals("page")) { long[] r = vec(b.get("refs"), 200, 0, 999);
                    sb.append("page ").append(word(b.get("algo"), "[A-Z]{3,5}")).append(' ').append(num(b.get("frames"), 1, 8)).append(' ').append(r.length).append(' '); join(sb, r); }
                else if (sim.equals("table")) { long[] f = vec(b.get("frames"), 64, -1, 100000), a = vec(b.get("addrs"), 64, 0, 100000000);
                    sb.append("table ").append(num(b.get("pagesize"), 1, 65536)).append(' ').append(f.length).append(' '); join(sb, f); sb.append(a.length).append(' '); join(sb, a); }
                else throw bad("Unknown memory simulation");
                break; }
            case "disk": {
                long[] r = vec(b.get("requests"), 64, 0, 100000);
                sb.append(word(b.get("algo"), "[A-Z]{4,5}")).append(' ').append(num(b.get("head"), 0, 100000)).append(' ').append(num(b.get("max"), 1, 100000)).append(' ')
                  .append(word(b.containsKey("direction") ? b.get("direction") : "up", "up|down")).append(' ').append(r.length).append(' '); join(sb, r);
                break; }
            default: throw new ApiException(404, "Unknown OS module");
        }
        return sb.toString();
    }
    public static Map<String, Object> run(long userId, String module, Map<String, Object> body) {
        String input = translate(module, body), label = label(module, body);
        String out = OsBridge.run(module, input);
        Object parsed;
        try { parsed = Json.parse(out); } catch (ApiException e) { throw new ApiException(500, "Engine returned invalid output"); }
        if (parsed instanceof Map && ((Map<?, ?>) parsed).containsKey("error")) throw new ApiException(400, String.valueOf(((Map<?, ?>) parsed).get("error")));
        Db.exec("INSERT INTO os_simulations(sim_id, user_id, module, algorithm, input_text, summary) VALUES (seq_os_sim.NEXTVAL, ?, ?, ?, ?, ?)",
                userId, module, label, input.length() > 2000 ? input.substring(0, 2000) : input, "Run via API");
        return Json.map("module", module, "result", parsed);
    }
}
