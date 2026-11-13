package gasgrid.controller;

import com.sun.net.httpserver.HttpExchange;
import gasgrid.model.*;
import gasgrid.service.AuthService;
import gasgrid.util.*;
import java.io.IOException;
import java.util.Map;
import static gasgrid.util.Json.map;

public final class AuthController {
    private AuthController() {}
    public static void handle(HttpExchange ex, String[] s, String m) throws IOException {
        if (s[0].equals("me")) { Session x = Http.require(ex); Http.send(ex, 200, map("user_id", x.userId, "name", x.name, "role", x.role)); return; }
        if (!m.equals("POST") || s.length < 2) throw new ApiException(405, "Use POST /api/auth/{login|register|logout|forgot|reset}");
        switch (s[1]) {
            case "login": {
                Map<String, Object> b = Http.body(ex);
                User u = AuthService.login(Http.req(b, "email"), Http.req(b, "password"));
                Session x = Http.login(ex, u); Http.send(ex, 200, map("name", u.name, "role", u.role, "user_id", x.userId)); return; }
            case "logout": Http.logout(ex); Http.send(ex, 200, map("ok", true)); return;
            case "register": {
                Map<String, Object> b = Http.body(ex);
                long id = AuthService.register(Http.req(b, "name"), Http.req(b, "email"), Http.str(b, "phone"), Http.str(b, "city"), Http.req(b, "password"));
                Http.send(ex, 201, map("user_id", id)); return; }
            case "forgot": {
                String t = AuthService.forgot(Http.req(Http.body(ex), "email"));
                Http.send(ex, 200, t == null ? map("ok", true) : map("ok", true, "demo_reset_token", t)); return; }   // demo: no e-mail server
            case "reset": { Map<String, Object> b = Http.body(ex); AuthService.reset(Http.req(b, "token"), Http.req(b, "password")); Http.send(ex, 200, map("ok", true)); return; }
            default: throw new ApiException(404, "Not found");
        }
    }
}
