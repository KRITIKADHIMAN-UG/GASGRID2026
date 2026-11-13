package gasgrid.dao;

import gasgrid.model.Session;
import gasgrid.util.*;
import java.sql.Types;
import java.util.*;

public final class BookingDao {
    private BookingDao() {}
    public static List<Map<String, Object>> list(Session s, String status, String q, int page, int size) {
        StringBuilder sql = new StringBuilder("SELECT booking_id, customer, address, wh_city, cyl_type, price, status, booked_at, delivery_status, worker_name, vehicle_reg FROM vw_booking_details WHERE 1=1");
        List<Object> p = new ArrayList<Object>();
        if ("CUSTOMER".equals(s.role)) { sql.append(" AND user_id = ?"); p.add(s.userId); }
        if (status != null && !status.isEmpty()) { sql.append(" AND status = ?"); p.add(status.toUpperCase()); }
        if (q != null && !q.isEmpty()) { sql.append(" AND (LOWER(customer) LIKE ? OR TO_CHAR(booking_id) = ?)"); p.add("%" + q.toLowerCase() + "%"); p.add(q); }
        sql.append(" ORDER BY booking_id DESC");
        return Paging.page(sql.toString(), page, size, p.toArray());
    }
    /** Both cylinder types with their own price and availability. */
    public static List<Map<String, Object>> types() { return Db.query("SELECT cyl_type, description, price, available FROM vw_inventory_by_type ORDER BY cyl_type"); }
    public static Map<String, Object> track(long id, Session s) {
        String sql = "SELECT booking_id, status, delivery_status, worker_name, vehicle_reg, wh_city, booked_at FROM vw_booking_details WHERE booking_id = ?";
        return "CUSTOMER".equals(s.role) ? Db.one(sql + " AND user_id = ?", id, s.userId) : Db.one(sql, id);
    }
    public static List<Map<String, Object>> connections(long userId) { return Db.query("SELECT conn_id, address, city, pin, status FROM connections WHERE user_id = ?", userId); }
    public static void addConnection(long userId, String address, String city, String pin) {
        Db.exec("INSERT INTO connections(conn_id, user_id, address, city, pin) VALUES (seq_connections.NEXTVAL, ?, ?, ?, ?)", userId, address, city, pin);
    }
    public static Object[] book(long user, long conn, String type) { return Db.call("{call sp_book_cylinder(?,?,?,?,?)}", new Object[]{user, conn, type}, Types.NUMERIC, Types.VARCHAR); }
    public static String cancel(long booking, long user, String role) { return (String) Db.call("{call sp_cancel_booking(?,?,?,?)}", new Object[]{booking, user, role}, Types.VARCHAR)[0]; }
    public static String updateStatus(long booking, String status) { return (String) Db.call("{call sp_update_delivery(?,?,?)}", new Object[]{booking, status}, Types.VARCHAR)[0]; }
}
