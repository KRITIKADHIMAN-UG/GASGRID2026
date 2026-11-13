package gasgrid.util;

import com.sun.net.httpserver.HttpExchange;
import gasgrid.model.Session;
import gasgrid.model.User;
import java.io.*;
import java.math.BigDecimal;
import java.nio.charset.StandardCharsets;
import java.util.*;
import java.util.concurrent.ConcurrentHashMap;

/** Request/response helpers, in-memory sessions and role checks. */
public final class Http {
    private Http() {}
    private static final Map<String, Session> SESSIONS = new ConcurrentHashMap<String, Session>();
    private static final long TTL = 8L * 3600 * 1000;

    @SuppressWarnings("unchecked")
    public static Map<String, Object> body(HttpExchange ex) throws IOException {
        ByteArrayOutputStream bo = new ByteArrayOutputStream(); byte[] buf = new byte[4096]; int n, total = 0;
        InputStream in = ex.getRequestBody();
        while ((n = in.read(buf)) > 0) { total += n; if (total > 65536) throw new ApiException(413, "Request too large"); bo.write(buf, 0, n); }
        String t = new String(bo.toByteArray(), StandardCharsets.UTF_8).trim();
        if (t.isEmpty()) return new LinkedHashMap<String, Object>();
        Object o = Json.parse(t);
        if (!(o instanceof Map)) throw new ApiException(400, "JSON object expected");
        return (Map<String, Object>) o;
    }
    public static void send(HttpExchange ex, int code, Object o) throws IOException {
        byte[] b = Json.stringify(o).getBytes(StandardCharsets.UTF_8);
        ex.getResponseHeaders().set("Content-Type", "application/json; charset=utf-8");
        ex.getResponseHeaders().set("Cache-Control", "no-store");
        ex.getResponseHeaders().set("X-Content-Type-Options", "nosniff");
        ex.sendResponseHeaders(code, b.length);
        try (OutputStream os = ex.getResponseBody()) { os.write(b); }
    }
    public static Session session(HttpExchange ex) {
        String c = ex.getRequestHeaders().getFirst("Cookie"); if (c == null) return null;
        for (String part : c.split(";")) {
            String p = part.trim();
            if (p.startsWith("GG_SESSION=")) {
                Session s = SESSIONS.get(p.substring(11));
                if (s != null && s.expires > System.currentTimeMillis()) return s;
                if (s != null) SESSIONS.remove(s.token);
            }
        }
        return null;
    }
    /** Requires a logged-in user, optionally restricted to roles. */
    public static Session require(HttpExchange ex, String... roles) {
        Session s = session(ex);
        if (s == null) throw new ApiException(401, "Login required");
        if (roles.length > 0 && !Arrays.asList(roles).contains(s.role)) throw new ApiException(403, "Not allowed for your role");
        return s;
    }
    public static Session login(HttpExchange ex, User u) {
        Session s = new Session(Security.token(), u.id, u.role, u.name, System.currentTimeMillis() + TTL);
        SESSIONS.put(s.token, s);
        ex.getResponseHeaders().add("Set-Cookie", "GG_SESSION=" + s.token + "; Path=/; HttpOnly; SameSite=Strict; Max-Age=28800");
        return s;
    }
    public static void logout(HttpExchange ex) {
        Session s = session(ex); if (s != null) SESSIONS.remove(s.token);
        ex.getResponseHeaders().add("Set-Cookie", "GG_SESSION=; Path=/; HttpOnly; SameSite=Strict; Max-Age=0");
    }
    public static String str(Map<String, Object> b, String k) { Object v = b.get(k); return v == null ? null : v.toString().trim(); }
    public static String req(Map<String, Object> b, String k) { String v = str(b, k); if (v == null || v.isEmpty()) throw new ApiException(400, "Missing field: " + k); return v; }
    public static long lng(Map<String, Object> b, String k) {
        Object v = b.get(k);
        try { if (v instanceof BigDecimal) return ((BigDecimal) v).longValueExact(); if (v != null) return Long.parseLong(v.toString().trim()); }
        catch (ArithmeticException e) { /* fall through */ } catch (NumberFormatException e) { /* fall through */ }
        throw new ApiException(400, "Invalid number: " + k);
    }
    public static int intParam(String q, String k, int def, int max) {
        if (q == null) return def;
        for (String p : q.split("&")) if (p.startsWith(k + "=")) { try { return Math.max(0, Math.min(max, Integer.parseInt(p.substring(k.length() + 1)))); } catch (NumberFormatException e) { return def; } }
        return def;
    }
    public static String param(String q, String k) {
        if (q == null) return null;
        for (String p : q.split("&")) if (p.startsWith(k + "=")) { try { return java.net.URLDecoder.decode(p.substring(k.length() + 1), "UTF-8").trim(); } catch (UnsupportedEncodingException e) { return null; } }
        return null;
    }
}
