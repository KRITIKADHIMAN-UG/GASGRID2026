package gasgrid;

import com.sun.net.httpserver.*;
import gasgrid.controller.*;
import gasgrid.util.*;
import java.io.*;
import java.net.InetSocketAddress;
import java.nio.file.*;
import java.util.concurrent.Executors;
import static gasgrid.util.Json.map;

/** GasGrid backend: REST API (/api/*) + static frontend, on localhost only. */
public final class Main {
    public static void main(String[] args) throws Exception {
        System.out.println("Initialising database ...");
        DbInit.run();
        HttpServer server = HttpServer.create(new InetSocketAddress("127.0.0.1", Config.PORT), 0);
        server.createContext("/api/", new HttpHandler() { public void handle(HttpExchange ex) throws IOException { api(ex); } });
        server.createContext("/", new HttpHandler() { public void handle(HttpExchange ex) throws IOException { staticFile(ex); } });
        server.setExecutor(Executors.newFixedThreadPool(8));
        server.start();
        System.out.println("GasGrid running at http://localhost:" + Config.PORT);
    }

    private static void api(HttpExchange ex) throws IOException {
        try {
            String path = ex.getRequestURI().getPath().substring(5), m = ex.getRequestMethod();
            String[] s = path.isEmpty() ? new String[0] : path.split("/");
            if (s.length == 0) throw new ApiException(404, "Not found");
            if (!m.equals("GET")) {   // CSRF defence in depth (cookie is also SameSite=Strict)
                String ct = ex.getRequestHeaders().getFirst("Content-Type");
                if (ct == null || !ct.toLowerCase().startsWith("application/json")) throw new ApiException(415, "Content-Type must be application/json");
            }
            switch (s[0]) {
                case "auth": case "me": AuthController.handle(ex, s, m); break;
                case "bookings": BookingController.handle(ex, s, m); break;
                case "service": ServiceController.handle(ex, s, m); break;
                case "account": AccountController.handle(ex, s, m); break;
                case "admin": AdminController.handle(ex, s, m); break;
                case "os": OsController.handle(ex, s, m); break;
                default: throw new ApiException(404, "Not found");
            }
        } catch (ApiException e) { Http.send(ex, e.code, map("error", e.getMessage())); }
        catch (Exception e) { e.printStackTrace(); Http.send(ex, 500, map("error", "Server error")); }
    }

    private static void staticFile(HttpExchange ex) throws IOException {
        Path base = Paths.get(Config.WEB_DIR).toAbsolutePath().normalize();
        String p = ex.getRequestURI().getPath(); if (p.equals("/")) p = "/index.html";
        Path f = base.resolve(p.substring(1)).normalize();   // normalize + startsWith blocks ../ traversal
        if (!ex.getRequestMethod().equals("GET") || !f.startsWith(base) || !Files.isRegularFile(f)) { byte[] b = "Not found".getBytes("UTF-8"); ex.sendResponseHeaders(404, b.length); ex.getResponseBody().write(b); ex.close(); return; }
        String n = f.getFileName().toString().toLowerCase();
        String ct = n.endsWith(".html") ? "text/html; charset=utf-8" : n.endsWith(".js") ? "application/javascript" : n.endsWith(".css") ? "text/css" : n.endsWith(".png") ? "image/png" : n.endsWith(".svg") ? "image/svg+xml" : "application/octet-stream";
        byte[] data = Files.readAllBytes(f);
        ex.getResponseHeaders().set("Content-Type", ct); ex.getResponseHeaders().set("X-Content-Type-Options", "nosniff"); ex.getResponseHeaders().set("X-Frame-Options", "DENY");
        ex.sendResponseHeaders(200, data.length);
        try (OutputStream os = ex.getResponseBody()) { os.write(data); }
    }
}
