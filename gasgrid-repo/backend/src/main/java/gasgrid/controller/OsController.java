package gasgrid.controller;

import com.sun.net.httpserver.HttpExchange;
import gasgrid.model.Session;
import gasgrid.service.OsService;
import gasgrid.util.*;
import java.io.IOException;

/** POST /api/os/{cpu|process|sync|deadlock|memory|disk}  and  GET /api/os/history */
public final class OsController {
    private OsController() {}
    public static void handle(HttpExchange ex, String[] s, String m) throws IOException {
        Session u = Http.require(ex); String q = ex.getRequestURI().getRawQuery();
        if (s.length == 2 && s[1].equals("history") && m.equals("GET")) {
            int page = Http.intParam(q, "page", 0, 100000), size = Http.intParam(q, "size", 15, 100);
            String sql = "SELECT sim_id, module, algorithm, summary, is_sample, created_at FROM os_simulations" + (u.role.equals("ADMIN") ? "" : " WHERE user_id = ?") + " ORDER BY sim_id DESC";
            Http.send(ex, 200, u.role.equals("ADMIN") ? Paging.page(sql, page, size) : Paging.page(sql, page, size, u.userId)); return;
        }
        if (s.length == 2 && m.equals("POST") && OsBridge.known(s[1])) { Http.send(ex, 200, OsService.run(u.userId, s[1], Http.body(ex))); return; }
        throw new ApiException(404, "Unknown OS module or method");
    }
}
