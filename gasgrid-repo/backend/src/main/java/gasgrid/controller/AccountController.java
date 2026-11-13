package gasgrid.controller;

import com.sun.net.httpserver.HttpExchange;
import gasgrid.dao.AccountDao;
import gasgrid.model.Session;
import gasgrid.service.BookingService;
import gasgrid.util.*;
import java.io.IOException;
import java.util.Map;
import static gasgrid.util.Json.map;

/** Rewards, notifications and family connections for the logged-in customer. */
public final class AccountController {
    private AccountController() {}
    public static void handle(HttpExchange ex, String[] s, String m) throws IOException {
        Session u = Http.require(ex); String a = s.length > 1 ? s[1] : "";
        if (a.equals("notifications")) {
            if (m.equals("GET")) { Http.send(ex, 200, AccountDao.notifications(u.userId)); return; }
            if (m.equals("POST")) { AccountDao.markRead(u.userId); Http.send(ex, 200, map("ok", true)); return; }
        }
        Http.require(ex, "CUSTOMER");
        if (a.equals("rewards") && m.equals("GET")) { Http.send(ex, 200, map("balance", AccountDao.balance(u.userId), "history", AccountDao.rewards(u.userId))); return; }
        if (a.equals("redeem") && m.equals("POST")) {
            String st = AccountDao.redeem(u.userId, Http.lng(Http.body(ex), "points")); int code = st.equals("INSUFFICIENT_POINTS") ? 409 : BookingService.http(st, 200);
            Http.send(ex, code, code == 200 ? map("status", st) : map("error", st)); return;
        }
        if (a.equals("family")) {
            if (m.equals("GET")) { Http.send(ex, 200, AccountDao.family(u.userId)); return; }
            if (m.equals("POST")) {
                Map<String, Object> b = Http.body(ex); String name = Http.req(b, "name"), rel = Http.req(b, "relation");
                if (name.length() < 2 || name.length() > 80 || rel.length() > 20) throw new ApiException(400, "Invalid name or relation");
                int n = AccountDao.addFamily(u.userId, Http.lng(b, "connection_id"), name, rel, b.containsKey("can_book") && Http.lng(b, "can_book") == 1 ? 1 : 0);
                if (n == 0) throw new ApiException(403, "Connection does not belong to you");
                Http.send(ex, 201, map("ok", true)); return;
            }
            if (s.length == 3 && m.equals("PUT")) { /* handled via POST below for clients without PUT */ }
        }
        if (a.equals("family-permission") && m.equals("POST")) {
            Map<String, Object> b = Http.body(ex);
            int n = AccountDao.setPermission(u.userId, Http.lng(b, "family_id"), Http.lng(b, "can_book") == 1 ? 1 : 0);
            if (n == 0) throw new ApiException(404, "Family member not found"); Http.send(ex, 200, map("ok", true)); return;
        }
        throw new ApiException(405, "Method not allowed");
    }
}
