package gasgrid.service;

import gasgrid.util.ApiException;
import java.util.*;

public final class BookingService {
    private BookingService() {}
    private static final Set<String> TYPES = new HashSet<String>(Arrays.asList("14.2KG", "19KG"));
    private static final Set<String> STATUSES = new HashSet<String>(Arrays.asList("SCHEDULED", "OUT_FOR_DELIVERY", "DELIVERED"));
    public static String type(String t) { if (t == null) return "14.2KG"; String u = t.toUpperCase(); if (!TYPES.contains(u)) throw new ApiException(400, "Cylinder type must be 14.2KG (domestic) or 19KG (commercial)"); return u; }
    public static String nextStatus(String s) { String u = s == null ? "" : s.toUpperCase(); if (!STATUSES.contains(u)) throw new ApiException(400, "Status must be SCHEDULED, OUT_FOR_DELIVERY or DELIVERED"); return u; }
    /** Maps stored-procedure result codes to HTTP status codes. */
    public static int http(String code, int okCode) {
        switch (code) {
            case "OK": return okCode;
            case "NOT_FOUND": case "NOT_FOUND_OR_DONE": return 404;
            case "FORBIDDEN": return 403;
            case "INVALID_CONNECTION": case "INVALID_TYPE": case "INVALID_PRICE": case "INVALID_POINTS": case "INVALID_QTY": return 400;
            default: return 409;   // DUPLICATE_BOOKING, OUT_OF_STOCK, NOT_CANCELLABLE, INVALID_TRANSITION, ...
        }
    }
}
