package gasgrid.util;

import java.util.*;

/** Oracle 11g-compatible pagination (ROWNUM). */
public final class Paging {
    private Paging() {}
    public static List<Map<String, Object>> page(String sql, int page, int size, Object... params) {
        size = Math.max(1, Math.min(size, 100));
        Object[] p = Arrays.copyOf(params, params.length + 2);
        p[params.length] = (page + 1) * size; p[params.length + 1] = page * size;
        List<Map<String, Object>> rows = Db.query("SELECT * FROM (SELECT a.*, ROWNUM rn__ FROM (" + sql + ") a WHERE ROWNUM <= ?) WHERE rn__ > ?", p);
        for (Map<String, Object> r : rows) r.remove("rn__");
        return rows;
    }
}
