package gasgrid.util;

import java.security.GeneralSecurityException;
import java.security.MessageDigest;
import java.security.SecureRandom;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

/** Password hashing (PBKDF2-HMAC-SHA256 with per-password salt) and random tokens. */
public final class Security {
    private Security() {}
    private static final SecureRandom RND = new SecureRandom();
    private static final int ITER = 65000;

    public static String hash(String password) { byte[] salt = new byte[16]; RND.nextBytes(salt); return "pbkdf2$" + ITER + "$" + hex(salt) + "$" + hex(derive(password, salt, ITER)); }
    public static boolean verify(String password, String stored) {
        if (stored == null) return false;
        String[] p = stored.split("\\$");
        if (p.length != 4 || !p[0].equals("pbkdf2")) return false;
        byte[] want = unhex(p[3]), got = derive(password, unhex(p[2]), Integer.parseInt(p[1]));
        return MessageDigest.isEqual(want, got);   // constant-time compare
    }
    public static String token() { byte[] b = new byte[24]; RND.nextBytes(b); return hex(b); }
    public static void checkPassword(String pw) {
        if (pw == null || pw.length() < 8 || !pw.matches(".*[A-Za-z].*") || !pw.matches(".*[0-9].*"))
            throw new ApiException(400, "Password must be at least 8 characters with letters and digits");
    }
    private static byte[] derive(String pw, byte[] salt, int iter) {
        try { return SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256").generateSecret(new PBEKeySpec(pw.toCharArray(), salt, iter, 256)).getEncoded(); }
        catch (GeneralSecurityException e) { throw new IllegalStateException(e); }
    }
    private static String hex(byte[] b) { StringBuilder s = new StringBuilder(); for (byte x : b) s.append(String.format("%02x", x)); return s.toString(); }
    private static byte[] unhex(String h) { byte[] b = new byte[h.length() / 2]; for (int i = 0; i < b.length; i++) b[i] = (byte) Integer.parseInt(h.substring(2 * i, 2 * i + 2), 16); return b; }
}
