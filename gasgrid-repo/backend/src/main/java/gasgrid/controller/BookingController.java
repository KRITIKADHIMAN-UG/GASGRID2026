package gasgrid.controller;

import com.sun.net.httpserver.HttpExchange;
import gasgrid.dao.BookingDao;
import gasgrid.model.Session;
import gasgrid.service.BookingService;
import gasgrid.util.*;
import java.io.IOException;
import java.util.Map;
import static gasgrid.util.Json.map;

public final class BookingController {
    private BookingController() {}
    public static void handle(HttpExchange ex, String[] s, String m) throws IOException {
        Session u = Http.require(ex); String q = ex.getRequestURI().getRawQuery();
        if (s.length == 1) {
            if (m.equals("GET")) { Http.send(ex, 200, BookingDao.list(u, Http.param(q, "status"), Http.param(q, "q"), Http.intParam(q, "page", 0, 100000), Http.intParam(q, "size", 15, 100))); return; }
            if (m.equals("POST")) {
                Http.require(ex, "CUSTOMER"); Map<String, Object> b = Http.body(ex);
                Object[] r = BookingDao.book(u.userId, Http.lng(b, "connection_id"), BookingService.type(Http.str(b, "cyl_type")));
                String st = String.valueOf(r[1]); int code = BookingService.http(st, 201);
                Http.send(ex, code, code == 201 ? map("status", st, "booking_id", r[0]) : map("error", st)); return;
            }
        } else if (s[1].equals("types") && m.equals("GET")) {
            Http.send(ex, 200, BookingDao.types()); return;
        } else if (s[1].equals("connections")) {
            Http.require(ex, "CUSTOMER");
            if (m.equals("GET")) { Http.send(ex, 200, BookingDao.connections(u.userId)); return; }
            if (m.equals("POST")) {
                Map<String, Object> b = Http.body(ex); String pin = Http.req(b, "pin"), addr = Http.req(b, "address");
                if (!pin.matches("^[0-9]{6}$")) throw new ApiException(400, "PIN must be 6 digits");
                if (addr.length() < 5 || addr.length() > 200) throw new ApiException(400, "Address must be 5-200 characters");
                BookingDao.addConnection(u.userId, addr, Http.str(b, "city"), pin); Http.send(ex, 201, map("ok", true)); return;
            }
        } else {
            long id; try { id = Long.parseLong(s[1]); } catch (NumberFormatException e) { throw new ApiException(404, "Not found"); }
            String a = s.length > 2 ? s[2] : "";
            if (a.equals("track") && m.equals("GET")) { Map<String, Object> t = BookingDao.track(id, u); if (t == null) throw new ApiException(404, "Booking not found"); Http.send(ex, 200, t); return; }
            if (a.equals("cancel") && m.equals("POST")) {
                Http.require(ex, "CUSTOMER", "ADMIN"); String st = BookingDao.cancel(id, u.userId, u.role); int code = BookingService.http(st, 200);
                Http.send(ex, code, code == 200 ? map("status", "CANCELLED") : map("error", st)); return;
            }
            if (a.equals("status") && m.equals("POST")) {
                Http.require(ex, "DISTRIBUTOR", "ADMIN"); String ns = BookingService.nextStatus(Http.req(Http.body(ex), "status"));
                String st = BookingDao.updateStatus(id, ns); int code = BookingService.http(st, 200);
                Http.send(ex, code, code == 200 ? map("status", ns) : map("error", st)); return;
            }
        }
        throw new ApiException(405, "Method not allowed");
    }
}
