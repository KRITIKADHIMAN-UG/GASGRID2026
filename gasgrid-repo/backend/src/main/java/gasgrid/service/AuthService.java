package gasgrid.service;

import gasgrid.dao.UserDao;
import gasgrid.model.User;
import gasgrid.util.*;
import java.util.*;
import java.util.concurrent.ConcurrentHashMap;

public final class AuthService {
    private AuthService() {}
    private static final Map<String, long[]> FAILS = new ConcurrentHashMap<String, long[]>();   // email -> {count, windowStart}
    private static final long WINDOW = 15L * 60 * 1000;

    public static User login(String email, String pw) {
        String key = email.toLowerCase(); long now = System.currentTimeMillis(); long[] f = FAILS.get(key);
        if (f != null && now - f[1] >= WINDOW) { FAILS.remove(key); f = null; }
        if (f != null && f[0] >= 5) throw new ApiException(429, "Too many failed attempts. Try again in 15 minutes.");
        User u = UserDao.byEmail(email);
        if (u == null || !Security.verify(pw, u.pwHash)) {
            if (f == null) FAILS.put(key, new long[]{1, now}); else f[0]++;
            throw new ApiException(401, "Invalid email or password");
        }
        FAILS.remove(key);
        UserDao.audit(u.id, "LOGIN", u.role);
        return u;
    }
    public static long register(String name, String email, String phone, String city, String password) {
        if (name.length() < 2 || name.length() > 80) throw new ApiException(400, "Name must be 2-80 characters");
        if (!email.matches("^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$") || email.length() > 120) throw new ApiException(400, "Invalid email");
        if (phone != null && !phone.isEmpty() && !phone.matches("^[6-9][0-9]{9}$")) throw new ApiException(400, "Phone must be a 10-digit Indian mobile number");
        Security.checkPassword(password);
        if (UserDao.byEmail(email) != null) throw new ApiException(409, "Email already registered");
        return UserDao.create(name, email, phone, city, Security.hash(password));
    }
    /** Returns a reset token, or null if the email is unknown. (Demo: token is returned instead of e-mailed.) */
    public static String forgot(String email) {
        User u = UserDao.byEmail(email); if (u == null) return null;
        String t = Security.token(); UserDao.createReset(u.id, t); return t;
    }
    public static void reset(String token, String newPassword) {
        Security.checkPassword(newPassword);
        long id = UserDao.consumeReset(token);
        if (id < 0) throw new ApiException(400, "Invalid or expired reset token");
        UserDao.setHash(id, Security.hash(newPassword));
        UserDao.audit(id, "PASSWORD_RESET", null);
    }
}
