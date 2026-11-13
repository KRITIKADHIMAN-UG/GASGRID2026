package gasgrid.dao;

import gasgrid.util.*;
import java.sql.Types;
import java.util.*;

public final class ServiceDao {
    private ServiceDao() {}
    public static List<Map<String, Object>> forCustomer(long uid) { return Db.query("SELECT sr_id, issue, priority, status, created_at, technician FROM vw_service_queue WHERE user_id = ? ORDER BY sr_id DESC", uid); }
    public static List<Map<String, Object>> queue(int page, int size) {
        return Paging.page("SELECT sr_id, customer, city, issue, priority, status, created_at, technician FROM vw_service_queue WHERE status <> 'DONE' ORDER BY priority, created_at", page, size);
    }
    public static void create(long uid, String issue, int priority) {
        Db.exec("INSERT INTO service_requests(sr_id, user_id, issue, priority) VALUES (seq_service.NEXTVAL, ?, ?, ?)", uid, issue, priority);
    }
    public static String assign(long id) { return (String) Db.call("{call sp_assign_technician(?,?)}", new Object[]{id}, Types.VARCHAR)[0]; }
    public static String complete(long id) { return (String) Db.call("{call sp_complete_service(?,?)}", new Object[]{id}, Types.VARCHAR)[0]; }
}
