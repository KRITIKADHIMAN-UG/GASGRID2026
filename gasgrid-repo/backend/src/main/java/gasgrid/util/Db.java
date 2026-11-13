package gasgrid.util;

import java.sql.*;
import java.util.*;

/** Thin JDBC helper. Each call opens a short-lived connection (fine for a demo; swap in a pool for production). */
public final class Db {
    private Db() {}
    static { try { Class.forName("oracle.jdbc.OracleDriver"); } catch (ClassNotFoundException e) { throw new IllegalStateException("Oracle JDBC driver not on classpath", e); } }

    public static Connection get() throws SQLException {
        Connection c = DriverManager.getConnection(Config.DB_URL, Config.DB_USER, Config.DB_PASS);
        c.setAutoCommit(false);
        return c;
    }
    private static void bind(PreparedStatement ps, Object[] p) throws SQLException { for (int i = 0; i < p.length; i++) ps.setObject(i + 1, p[i]); }

    public static List<Map<String, Object>> query(String sql, Object... p) {
        try (Connection c = get(); PreparedStatement ps = c.prepareStatement(sql)) {
            bind(ps, p);
            try (ResultSet rs = ps.executeQuery()) { return rows(rs); }
        } catch (SQLException e) { throw dbError(e); }
    }
    public static Map<String, Object> one(String sql, Object... p) { List<Map<String, Object>> r = query(sql, p); return r.isEmpty() ? null : r.get(0); }
    public static int exec(String sql, Object... p) {
        try (Connection c = get(); PreparedStatement ps = c.prepareStatement(sql)) {
            bind(ps, p); int n = ps.executeUpdate(); c.commit(); return n;
        } catch (SQLException e) { throw dbError(e); }
    }
    /** Calls "{call proc(?,?,..)}": inputs first, then OUT parameter types (java.sql.Types). Returns the OUT values. */
    public static Object[] call(String sql, Object[] in, int... outTypes) {
        try (Connection c = get(); CallableStatement cs = c.prepareCall(sql)) {
            for (int i = 0; i < in.length; i++) cs.setObject(i + 1, in[i]);
            for (int i = 0; i < outTypes.length; i++) cs.registerOutParameter(in.length + i + 1, outTypes[i]);
            cs.execute(); c.commit();
            Object[] out = new Object[outTypes.length];
            for (int i = 0; i < out.length; i++) out[i] = cs.getObject(in.length + i + 1);
            return out;
        } catch (SQLException e) { throw dbError(e); }
    }
    private static ApiException dbError(SQLException e) {
        System.err.println("DB error " + e.getErrorCode() + ": " + e.getMessage());
        if (e.getErrorCode() == 20001) return new ApiException(409, "Invalid status transition");
        if (e.getErrorCode() == 1) return new ApiException(409, "Already exists");
        return new ApiException(500, "Database error");
    }
    private static List<Map<String, Object>> rows(ResultSet rs) throws SQLException {
        ResultSetMetaData md = rs.getMetaData(); List<Map<String, Object>> out = new ArrayList<Map<String, Object>>();
        while (rs.next()) {
            Map<String, Object> m = new LinkedHashMap<String, Object>();
            for (int i = 1; i <= md.getColumnCount(); i++) {
                int t = md.getColumnType(i); Object v;
                if (t == Types.DATE || t == Types.TIMESTAMP) { Timestamp ts = rs.getTimestamp(i); v = ts == null ? null : ts.toString().substring(0, 19); }
                else v = rs.getObject(i);
                m.put(md.getColumnLabel(i).toLowerCase(), v);
            }
            out.add(m);
        }
        return out;
    }
}
