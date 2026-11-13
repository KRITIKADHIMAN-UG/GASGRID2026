package gasgrid.util;

import java.io.File;
import java.util.*;

/** Creates schema + sample data when missing, then sets demo/sample passwords. Safe to run on every start. */
public final class DbInit {
    private DbInit() {}
    private static final String[] FILES = {"01_create_tables.sql", "02_indexes_views.sql", "03_procedures_functions.sql", "04_triggers.sql", "05_seed_data.sql"};

    public static void run() throws Exception {
        for (String f : FILES) {
            File file = new File(Config.SQL_DIR, f);
            if (!file.exists()) throw new IllegalStateException("Missing SQL script: " + file.getAbsolutePath());
            System.out.println("Running " + f);
            SqlRunner.runScript(file);
        }
        // CREATE PROCEDURE/TRIGGER with errors does not throw in JDBC, so check explicitly
        List<Map<String, Object>> errs = Db.query("SELECT name, line, text FROM user_errors WHERE type IN ('PROCEDURE','FUNCTION','TRIGGER') ORDER BY name, sequence");
        if (!errs.isEmpty()) { for (Map<String, Object> e : errs) System.err.println("PL/SQL error in " + e.get("name") + " line " + e.get("line") + ": " + e.get("text")); throw new IllegalStateException("PL/SQL compilation errors, see log"); }
        seedPasswords();
    }
    private static void seedPasswords() {
        List<Map<String, Object>> rows = Db.query("SELECT user_id, email FROM users WHERE pw_hash = 'SEED'");
        if (rows.isEmpty()) return;
        String sample = Security.hash("sample123");
        Map<String, String> demo = new HashMap<String, String>();
        demo.put("admin@gasgrid.in", "admin123"); demo.put("distributor@gasgrid.in", "distributor123");
        demo.put("technician@gasgrid.in", "technician123"); demo.put("customer@gasgrid.in", "customer123");
        for (Map<String, Object> r : rows) {
            String pw = demo.get(r.get("email"));
            Db.exec("UPDATE users SET pw_hash = ? WHERE user_id = ?", pw == null ? sample : Security.hash(pw), r.get("user_id"));
        }
        System.out.println("Sample passwords initialised for " + rows.size() + " users");
    }
}
