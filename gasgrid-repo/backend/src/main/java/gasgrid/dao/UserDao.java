package gasgrid.dao;

import gasgrid.model.User;
import gasgrid.util.*;
import java.util.*;

public final class UserDao {
    private UserDao() {}
    private static User map(Map<String, Object> r) {
        User u = new User(); u.id = ((Number) r.get("user_id")).longValue(); u.name = (String) r.get("full_name");
        u.email = (String) r.get("email"); u.city = (String) r.get("city"); u.pwHash = (String) r.get("pw_hash"); u.role = (String) r.get("role"); return u;
    }
    public static User byEmail(String email) {
        Map<String, Object> r = Db.one("SELECT user_id, full_name, email, city, pw_hash, role FROM users WHERE LOWER(email) = LOWER(?)", email);
        return r == null ? null : map(r);
    }
    public static long create(String name, String email, String phone, String city, String hash) {
        long id = ((Number) Db.one("SELECT seq_users.NEXTVAL AS id FROM dual").get("id")).longValue();
        Db.exec("INSERT INTO users(user_id, full_name, email, phone, city, pw_hash, role, is_sample) VALUES (?,?,?,?,?,?,'CUSTOMER',0)", id, name, email, phone, city, hash);
        return id;
    }
    public static void setHash(long id, String hash) { Db.exec("UPDATE users SET pw_hash = ? WHERE user_id = ?", hash, id); }
    public static void createReset(long userId, String token) {
        Db.exec("INSERT INTO password_resets(token, user_id, expires_at) VALUES (?, ?, SYSDATE + 30/1440)", token, userId);
    }
    /** Returns the user id for a valid unused token and marks it used, else -1. */
    public static long consumeReset(String token) {
        Map<String, Object> r = Db.one("SELECT user_id FROM password_resets WHERE token = ? AND used = 0 AND expires_at > SYSDATE", token);
        if (r == null) return -1;
        Db.exec("UPDATE password_resets SET used = 1 WHERE token = ?", token);
        return ((Number) r.get("user_id")).longValue();
    }
    public static void audit(Long userId, String action, String details) {
        Db.exec("INSERT INTO audit_logs(log_id, user_id, action, details) VALUES (seq_audit.NEXTVAL, ?, ?, ?)", userId, action, details);
    }
    public static List<Map<String, Object>> list(String q, int page, int size) {
        if (q == null || q.isEmpty()) return Paging.page("SELECT user_id, full_name, email, role, city, is_sample FROM users ORDER BY user_id", page, size);
        return Paging.page("SELECT user_id, full_name, email, role, city, is_sample FROM users WHERE LOWER(full_name) LIKE ? OR LOWER(email) LIKE ? ORDER BY user_id", page, size, "%" + q.toLowerCase() + "%", "%" + q.toLowerCase() + "%");
    }
}
