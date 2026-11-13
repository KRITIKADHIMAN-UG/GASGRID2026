package gasgrid.util;

import java.io.*;
import java.nio.charset.StandardCharsets;
import java.sql.*;
import java.util.*;

/** Executes the .sql scripts (SQL*Plus style: ';' ends a statement, a lone '/' ends a PL/SQL block). Idempotent: "already exists" errors are skipped. */
public final class SqlRunner {
    private SqlRunner() {}
    private static final Set<Integer> SKIP = new HashSet<Integer>(Arrays.asList(955, 1408, 2260, 2261, 2275, 1430, 4081));

    public static void runScript(File f) throws IOException, SQLException {
        try (Connection c = Db.get(); Statement st = c.createStatement();
             BufferedReader r = new BufferedReader(new InputStreamReader(new FileInputStream(f), StandardCharsets.UTF_8))) {
            StringBuilder cur = new StringBuilder(); boolean block = false; String line;
            while ((line = r.readLine()) != null) {
                String t = line.trim();
                if (t.startsWith("--") || (cur.length() == 0 && t.isEmpty())) continue;
                if (cur.length() == 0) block = t.toUpperCase().matches("^(CREATE (OR REPLACE )?(TRIGGER|PROCEDURE|FUNCTION|PACKAGE).*|DECLARE.*|BEGIN.*)");
                if (block) {
                    if (t.equals("/")) { exec(st, cur.toString()); cur.setLength(0); } else cur.append(line).append('\n');
                } else {
                    cur.append(line).append('\n');
                    if (t.endsWith(";")) { String sql = cur.toString().trim(); exec(st, sql.substring(0, sql.length() - 1)); cur.setLength(0); }
                }
            }
            c.commit();
        }
    }
    private static void exec(Statement st, String sql) throws SQLException {
        try { st.execute(sql); }
        catch (SQLException e) {
            if (SKIP.contains(e.getErrorCode())) return;
            System.err.println("SQL failed (ORA-" + e.getErrorCode() + "): " + sql.substring(0, Math.min(80, sql.length())).replace('\n', ' '));
            throw e;
        }
    }
}
