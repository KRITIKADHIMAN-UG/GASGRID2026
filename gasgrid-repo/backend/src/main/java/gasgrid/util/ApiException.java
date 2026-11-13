package gasgrid.util;

/** Error that is safe to show to the client (HTTP status + message). */
public class ApiException extends RuntimeException {
    private static final long serialVersionUID = 1L;
    public final int code;
    public ApiException(int code, String message) { super(message); this.code = code; }
}
