package gasgrid.util;

/** Configuration from environment variables (see .env.example). */
public final class Config {
    private Config() {}
    private static String env(String k, String d) { String v = System.getenv(k); return v == null || v.isEmpty() ? d : v; }
    public static final String DB_URL  = env("GASGRID_DB_URL", "jdbc:oracle:thin:@//localhost:1521/XE");
    public static final String DB_USER = env("GASGRID_DB_USER", "gasgrid");
    public static final String DB_PASS = env("GASGRID_DB_PASS", "gasgrid");
    public static final String OS_DIR  = env("GASGRID_OS_DIR", "../os/bin");
    public static final String WEB_DIR = env("GASGRID_WEB_DIR", "../frontend");
    public static final String SQL_DIR = env("GASGRID_SQL_DIR", "../database");
    public static final int PORT = Integer.parseInt(env("GASGRID_PORT", "8080"));
}
