package gasgrid.controller;

import com.sun.net.httpserver.HttpExchange;
import gasgrid.dao.ServiceDao;
import gasgrid.model.Session;
import gasgrid.service.BookingService;
import gasgrid.util.*;
import java.io.IOException;
import java.util.Map;
import static gasgrid.util.Json.map;

public final class ServiceController {
    private ServiceController() {}
    public static void handle(HttpExchange ex, String[] s, String m) throws IOException {
        Session u = Http.require(ex, "CUSTOMER", "TECHNICIAN", "ADMIN"); String q = ex.getRequestURI().getRawQuery();
        if (s.length == 1 && m.equals("GET")) {
            Http.send(ex, 200, u.role.equals("CUSTOMER") ? ServiceDao.forCustomer(u.userId) : ServiceDao.queue(Http.intParam(q, "page", 0, 100000), Http.intParam(q, "size", 15, 100))); return;
        }
        if (s.length == 1 && m.equals("POST")) {
            Http.require(ex, "CUSTOMER"); Map<String, Object> b = Http.body(ex); String issue = Http.req(b, "issue"); long pr = b.containsKey("priority") ? Http.lng(b, "priority") : 2;
            if (issue.length() < 5 || issue.length() > 200) throw new ApiException(400, "Issue must be 5-200 characters");
            if (pr < 1 || pr > 3) throw new ApiException(400, "Priority must be 1 (high) to 3 (low)");
            ServiceDao.create(u.userId, issue, (int) pr); Http.send(ex, 201, map("ok", true)); return;
        }
        if (s.length == 3 && m.equals("POST")) {
            long id; try { id = Long.parseLong(s[1]); } catch (NumberFormatException e) { throw new ApiException(404, "Not found"); }
            String st;
            if (s[2].equals("assign")) { Http.require(ex, "ADMIN"); st = ServiceDao.assign(id); }
            else if (s[2].equals("complete")) { Http.require(ex, "TECHNICIAN", "ADMIN"); st = ServiceDao.complete(id); }
            else throw new ApiException(404, "Not found");
            int code = BookingService.http(st, 200); Http.send(ex, code, code == 200 ? map("status", st) : map("error", st)); return;
        }
        throw new ApiException(405, "Method not allowed");
    }
}
