package gasgrid.dao;

import gasgrid.util.*;
import java.sql.Types;
import java.util.*;

public final class ReportDao {
    private ReportDao() {}
    public static Map<String, Object> stats() {
        return Db.one("SELECT (SELECT COUNT(*) FROM users) AS users, (SELECT COUNT(*) FROM bookings) AS bookings, "
            + "(SELECT NVL(SUM(price), 0) FROM bookings WHERE status = 'DELIVERED') AS revenue, "
            + "(SELECT COUNT(*) FROM service_requests WHERE status <> 'DONE') AS open_service, "
            + "(SELECT SUM(qty_full - qty_reserved) FROM inventory) AS stock_available FROM dual");
    }
    public static List<Map<String, Object>> byCity() { return Db.query("SELECT city, total, delivered, cancelled, revenue FROM vw_city_bookings ORDER BY total DESC"); }
    public static List<Map<String, Object>> byStatus() { return Db.query("SELECT status, COUNT(*) AS cnt FROM bookings GROUP BY status HAVING COUNT(*) > 0 ORDER BY cnt DESC"); }
    public static List<Map<String, Object>> topCustomers() { return Db.query("SELECT * FROM (SELECT user_id, full_name, total_bookings, points_balance FROM vw_customer_summary ORDER BY points_balance DESC, total_bookings DESC) WHERE ROWNUM <= 10"); }
    public static List<Map<String, Object>> monthly() { return Db.query("SELECT TO_CHAR(booked_at, 'YYYY-MM') AS month, COUNT(*) AS bookings, NVL(SUM(CASE WHEN status = 'DELIVERED' THEN price END), 0) AS revenue FROM bookings GROUP BY TO_CHAR(booked_at, 'YYYY-MM') ORDER BY month"); }
    public static List<Map<String, Object>> inventoryStatus() { return Db.query("SELECT wh_id, wh_name, city, qty_full, qty_reserved, available FROM vw_inventory_status ORDER BY available"); }
    public static List<Map<String, Object>> batches(Integer wh, int page, int size) {
        if (wh == null) return Paging.page("SELECT inv_id, wh_id, cyl_type, batch_no, qty_full, qty_reserved FROM inventory ORDER BY inv_id", page, size);
        return Paging.page("SELECT inv_id, wh_id, cyl_type, batch_no, qty_full, qty_reserved FROM inventory WHERE wh_id = ? ORDER BY inv_id", page, size, wh);
    }
    public static List<Map<String, Object>> byType() { return Db.query("SELECT cyl_type, total, delivered, revenue FROM vw_type_bookings ORDER BY cyl_type"); }
    public static List<Map<String, Object>> inventoryByType() { return Db.query("SELECT cyl_type, description, price, qty_full, qty_reserved, available FROM vw_inventory_by_type ORDER BY cyl_type"); }
    public static String setPrice(String type, long price) { return (String) Db.call("{call sp_set_price(?,?,?)}", new Object[]{type, price}, Types.VARCHAR)[0]; }
    public static String restock(long inv, long qty) { return (String) Db.call("{call sp_restock(?,?,?)}", new Object[]{inv, qty}, Types.VARCHAR)[0]; }
    public static List<Map<String, Object>> vehicles() { return Db.query("SELECT v.veh_id, v.reg_no, w.city, v.capacity, v.status FROM vehicles v JOIN warehouses w ON w.wh_id = v.wh_id ORDER BY v.veh_id"); }
    public static List<Map<String, Object>> workers() { return Db.query("SELECT k.worker_id, k.full_name, k.phone, w.city, k.status FROM workers k JOIN warehouses w ON w.wh_id = k.wh_id ORDER BY k.worker_id"); }
    public static List<Map<String, Object>> audit(int page, int size) { return Paging.page("SELECT log_id, user_id, action, details, logged_at FROM audit_logs ORDER BY log_id DESC", page, size); }
}
