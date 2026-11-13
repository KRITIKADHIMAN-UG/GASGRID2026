package gasgrid.dao;

import gasgrid.util.*;
import java.sql.Types;
import java.util.*;

public final class AccountDao {
    private AccountDao() {}
    public static Object balance(long uid) { return Db.one("SELECT fn_points_balance(?) AS balance FROM dual", uid).get("balance"); }
    public static List<Map<String, Object>> rewards(long uid) { return Db.query("SELECT txn_id, points, txn_type, booking_id, created_at FROM rewards_txn WHERE user_id = ? ORDER BY txn_id DESC", uid); }
    public static String redeem(long uid, long pts) { return (String) Db.call("{call sp_redeem_points(?,?,?)}", new Object[]{uid, pts}, Types.VARCHAR)[0]; }
    public static List<Map<String, Object>> notifications(long uid) { return Db.query("SELECT * FROM (SELECT notif_id, message, is_read, created_at FROM notifications WHERE user_id = ? ORDER BY notif_id DESC) WHERE ROWNUM <= 30", uid); }
    public static void markRead(long uid) { Db.exec("UPDATE notifications SET is_read = 1 WHERE user_id = ?", uid); }
    public static List<Map<String, Object>> family(long uid) { return Db.query("SELECT f.fam_id, f.conn_id, f.member_name, f.relation, f.can_book FROM family_members f JOIN connections c ON c.conn_id = f.conn_id WHERE c.user_id = ? ORDER BY f.fam_id", uid); }
    public static int addFamily(long uid, long conn, String name, String relation, int canBook) {
        return Db.exec("INSERT INTO family_members(fam_id, conn_id, member_name, relation, can_book) SELECT seq_family.NEXTVAL, conn_id, ?, ?, ? FROM connections WHERE conn_id = ? AND user_id = ?", name, relation, canBook, conn, uid);
    }
    public static int setPermission(long uid, long fam, int canBook) {
        return Db.exec("UPDATE family_members SET can_book = ? WHERE fam_id = ? AND conn_id IN (SELECT conn_id FROM connections WHERE user_id = ?)", canBook, fam, uid);
    }
}
