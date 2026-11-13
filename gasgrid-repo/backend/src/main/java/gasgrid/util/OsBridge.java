package gasgrid.util;

import java.io.*;
import java.nio.charset.StandardCharsets;
import java.util.*;
import java.util.concurrent.TimeUnit;

/** Runs the compiled C engine. Only whitelisted executables, fixed argv, no shell, stdin/stdout only. */
public final class OsBridge {
    private OsBridge() {}
    private static final Map<String, String> MODULES = new HashMap<String, String>();
    static {
        MODULES.put("cpu", "cpu_scheduling"); MODULES.put("process", "process_management"); MODULES.put("sync", "synchronization");
        MODULES.put("deadlock", "deadlock_detection"); MODULES.put("memory", "memory_management"); MODULES.put("disk", "disk_scheduling");
    }
    public static boolean known(String module) { return MODULES.containsKey(module); }

    public static String run(String module, String stdin) {
        String exe = MODULES.get(module);
        if (exe == null) throw new ApiException(404, "Unknown OS module");
        File f = new File(Config.OS_DIR, exe + (System.getProperty("os.name").toLowerCase().contains("win") ? ".exe" : ""));
        if (!f.canExecute()) throw new ApiException(503, "OS engine not built. Run os/build.bat (or make in os/).");
        try {
            Process p = new ProcessBuilder(f.getAbsolutePath()).redirectErrorStream(true).start();
            try (OutputStream os = p.getOutputStream()) { os.write(stdin.getBytes(StandardCharsets.UTF_8)); }
            ByteArrayOutputStream bo = new ByteArrayOutputStream(); byte[] buf = new byte[8192]; int n;
            InputStream in = p.getInputStream();
            while ((n = in.read(buf)) > 0) { bo.write(buf, 0, n); if (bo.size() > 1 << 20) { p.destroyForcibly(); throw new ApiException(500, "Engine output too large"); } }
            if (!p.waitFor(5, TimeUnit.SECONDS)) { p.destroyForcibly(); throw new ApiException(500, "Engine timeout"); }
            return new String(bo.toByteArray(), StandardCharsets.UTF_8).trim();
        } catch (IOException e) { throw new ApiException(500, "Engine failed to start"); }
        catch (InterruptedException e) { Thread.currentThread().interrupt(); throw new ApiException(500, "Interrupted"); }
    }
}
