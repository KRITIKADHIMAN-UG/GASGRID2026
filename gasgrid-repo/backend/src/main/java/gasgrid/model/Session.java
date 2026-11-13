package gasgrid.model;

public class Session {
    public final String token; public final long userId; public final String role, name; public final long expires;
    public Session(String token, long userId, String role, String name, long expires) { this.token = token; this.userId = userId; this.role = role; this.name = name; this.expires = expires; }
}
