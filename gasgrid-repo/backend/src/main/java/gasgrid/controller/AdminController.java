package gasgrid.controller;

import com.sun.net.httpserver.HttpExchange;
import gasgrid.dao.*;
import gasgrid.service.BookingService;
import gasgrid.util.*;
import java.io.IOException;
import java.util.Map;
import static gasgrid.util.Json.map;

/** /api/admin/*: stats, reports, users, audit log, inventory, vehicles, workers. */
public final class AdminController {
    private AdminController() {}
    public static void handle(HttpExchange ex, String[] s, String m) throws IOException {
        String a = s.length > 1 ? s[1] : "", q = ex.getRequestURI().getRawQuery();
        int page = Http.intParam(q, "page", 0, 100000), size = Http.intParam(q, "size", 15, 100);
        if (a.equals("inventory") || a.equals("restock") || a.equals("vehicles") || a.equals("workers")) {
            Http.require(ex, "ADMIN", "DISTRIBUTOR");
            if (a.equals("inventory") && m.equals("GET")) {
                String wh = Http.param(q, "warehouse");
                if (s.length > 2 && s[2].equals("types")) { Http.send(ex, 200, ReportDao.inventoryByType()); return; }
                if (s.length > 2 && s[2].equals("summary")) { Http.send(ex, 200, ReportDao.inventoryStatus()); return; }
                Http.send(ex, 200, ReportDao.batches(wh == null ? null : Integer.valueOf(wh.replaceAll("[^0-9]", "0")), page, size)); return;
            }
            if (a.equals("restock") && m.equals("POST")) {
                Map<String, Object> b = Http.body(ex); String st = ReportDao.restock(Http.lng(b, "inv_id"), Http.lng(b, "qty")); int code = BookingService.http(st, 200);
                Http.send(ex, code, code == 200 ? map("status", st) : map("error", st)); return;
            }
            if (a.equals("vehicles") && m.equals("GET")) { Http.send(ex, 200, ReportDao.vehicles()); return; }
            if (a.equals("workers") && m.equals("GET")) { Http.send(ex, 200, ReportDao.workers()); return; }
            throw new ApiException(405, "Method not allowed");
        }
        Http.require(ex, "ADMIN");
        if (a.equals("price") && m.equals("POST")) {
            Map<String, Object> b = Http.body(ex); String type = BookingService.type(Http.req(b, "cyl_type")); String st = ReportDao.setPrice(type, Http.lng(b, "price")); int code = BookingService.http(st, 200);
            Http.send(ex, code, code == 200 ? map("status", st) : map("error", st)); return;
        }
        if (!m.equals("GET")) throw new ApiException(405, "Method not allowed");
        switch (a) {
            case "stats": Http.send(ex, 200, ReportDao.stats()); return;
            case "users": Http.send(ex, 200, UserDao.list(Http.param(q, "q"), page, size)); return;
            case "audit": Http.send(ex, 200, ReportDao.audit(page, size)); return;
            case "reports": {
                String r = s.length > 2 ? s[2] : "";
                switch (r) {
                    case "city": Http.send(ex, 200, ReportDao.byCity()); return;
                    case "types": Http.send(ex, 200, ReportDao.byType()); return;
                    case "status": Http.send(ex, 200, ReportDao.byStatus()); return;
                    case "top": Http.send(ex, 200, ReportDao.topCustomers()); return;
                    case "monthly": Http.send(ex, 200, ReportDao.monthly()); return;
                    default: throw new ApiException(404, "Unknown report");
                }
            }
            default: throw new ApiException(404, "Not found");
        }
    }
}
