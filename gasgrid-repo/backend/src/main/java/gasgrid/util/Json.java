package gasgrid.util;

import java.math.BigDecimal;
import java.util.*;

/** Minimal JSON parser/serializer (no external libraries). */
public final class Json {
    private final String s; private int i; private int depth;
    private Json(String s) { this.s = s; }

    public static Object parse(String text) {
        Json j = new Json(text); j.ws(); Object o = j.val(); j.ws();
        if (j.i != text.length()) throw new ApiException(400, "Invalid JSON");
        return o;
    }
    private ApiException bad() { return new ApiException(400, "Invalid JSON"); }
    private void ws() { while (i < s.length() && Character.isWhitespace(s.charAt(i))) i++; }
    private boolean peek(char c) { return i < s.length() && s.charAt(i) == c; }
    private void expect(char c) { if (!peek(c)) throw bad(); i++; }

    private Object val() {
        ws(); if (i >= s.length()) throw bad();
        if (++depth > 20) throw bad();
        char c = s.charAt(i); Object r;
        if (c == '{') {
            i++; Map<String, Object> m = new LinkedHashMap<String, Object>(); ws();
            if (peek('}')) { i++; } else {
                while (true) { ws(); String k = str(); ws(); expect(':'); m.put(k, val()); ws(); if (peek(',')) { i++; continue; } expect('}'); break; }
            }
            r = m;
        } else if (c == '[') {
            i++; List<Object> l = new ArrayList<Object>(); ws();
            if (peek(']')) { i++; } else {
                while (true) { l.add(val()); ws(); if (peek(',')) { i++; continue; } expect(']'); break; }
            }
            r = l;
        } else if (c == '"') r = str();
        else if (s.startsWith("true", i)) { i += 4; r = Boolean.TRUE; }
        else if (s.startsWith("false", i)) { i += 5; r = Boolean.FALSE; }
        else if (s.startsWith("null", i)) { i += 4; r = null; }
        else r = num();
        depth--; return r;
    }
    private Object num() {
        int st = i; while (i < s.length() && "+-0123456789.eE".indexOf(s.charAt(i)) >= 0) i++;
        if (st == i) throw bad();
        try { return new BigDecimal(s.substring(st, i)); } catch (NumberFormatException e) { throw bad(); }
    }
    private String str() {
        expect('"'); StringBuilder b = new StringBuilder();
        while (true) {
            if (i >= s.length()) throw bad();
            char c = s.charAt(i++);
            if (c == '"') break;
            if (c != '\\') { b.append(c); continue; }
            if (i >= s.length()) throw bad();
            char e = s.charAt(i++);
            switch (e) {
                case 'n': b.append('\n'); break; case 't': b.append('\t'); break; case 'r': b.append('\r'); break;
                case 'b': b.append('\b'); break; case 'f': b.append('\f'); break;
                case 'u': if (i + 4 > s.length()) throw bad(); b.append((char) Integer.parseInt(s.substring(i, i + 4), 16)); i += 4; break;
                default: b.append(e);
            }
        }
        return b.toString();
    }

    public static String stringify(Object o) { StringBuilder b = new StringBuilder(); write(b, o); return b.toString(); }
    @SuppressWarnings("unchecked")
    private static void write(StringBuilder b, Object o) {
        if (o == null) b.append("null");
        else if (o instanceof String) quote(b, (String) o);
        else if (o instanceof BigDecimal) b.append(((BigDecimal) o).toPlainString());
        else if (o instanceof Number || o instanceof Boolean) b.append(o);
        else if (o instanceof Map) {
            b.append('{'); boolean f = true;
            for (Map.Entry<String, Object> e : ((Map<String, Object>) o).entrySet()) { if (!f) b.append(','); f = false; quote(b, e.getKey()); b.append(':'); write(b, e.getValue()); }
            b.append('}');
        } else if (o instanceof Iterable) {
            b.append('['); boolean f = true;
            for (Object x : (Iterable<Object>) o) { if (!f) b.append(','); f = false; write(b, x); }
            b.append(']');
        } else quote(b, o.toString());
    }
    private static void quote(StringBuilder b, String s) {
        b.append('"');
        for (int k = 0; k < s.length(); k++) {
            char c = s.charAt(k);
            switch (c) {
                case '"': b.append("\\\""); break; case '\\': b.append("\\\\"); break; case '\n': b.append("\\n"); break;
                case '\r': b.append("\\r"); break; case '\t': b.append("\\t"); break;
                default: if (c < 0x20) b.append(String.format("\\u%04x", (int) c)); else b.append(c);
            }
        }
        b.append('"');
    }
    public static Map<String, Object> map(Object... kv) {
        Map<String, Object> m = new LinkedHashMap<String, Object>();
        for (int k = 0; k + 1 < kv.length; k += 2) m.put((String) kv[k], kv[k + 1]);
        return m;
    }
}
