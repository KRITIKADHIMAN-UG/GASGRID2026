-- Stored procedures / functions. A line with a single "/" ends each PL/SQL block.
-- Deterministic helper used by the seed script: i-th item (0-based, wraps) of a comma list
CREATE OR REPLACE FUNCTION fn_pick(l IN VARCHAR2, i IN NUMBER) RETURN VARCHAR2 DETERMINISTIC IS
  n NUMBER := LENGTH(l) - LENGTH(REPLACE(l, ',')) + 1;
  k NUMBER := MOD(i, n) + 1; s NUMBER; e NUMBER;
BEGIN
  s := CASE k WHEN 1 THEN 1 ELSE INSTR(l, ',', 1, k - 1) + 1 END;
  e := INSTR(l, ',', 1, k);
  IF e = 0 THEN e := LENGTH(l) + 1; END IF;
  RETURN SUBSTR(l, s, e - s);
END;
/
CREATE OR REPLACE FUNCTION fn_cylinder_price(p_type IN VARCHAR2) RETURN NUMBER IS
  v NUMBER;
BEGIN
  SELECT price INTO v FROM cylinder_types WHERE cyl_type = p_type;   -- separate price per cylinder type
  RETURN v;
EXCEPTION WHEN NO_DATA_FOUND THEN RETURN NULL;
END;
/
CREATE OR REPLACE PROCEDURE sp_set_price(p_type IN VARCHAR2, p_price IN NUMBER, p_status OUT VARCHAR2) IS
BEGIN
  IF p_price IS NULL OR p_price < 100 OR p_price > 10000 THEN p_status := 'INVALID_PRICE'; RETURN; END IF;
  UPDATE cylinder_types SET price = p_price WHERE cyl_type = p_type;
  IF SQL%ROWCOUNT = 0 THEN p_status := 'NOT_FOUND'; ROLLBACK; ELSE COMMIT; p_status := 'OK'; END IF;
END;
/
CREATE OR REPLACE FUNCTION fn_points_balance(p_user IN NUMBER) RETURN NUMBER IS
  v NUMBER;
BEGIN
  SELECT NVL(SUM(CASE WHEN txn_type = 'EARN' THEN points ELSE -points END), 0) INTO v FROM rewards_txn WHERE user_id = p_user;
  RETURN v;
END;
/
CREATE OR REPLACE PROCEDURE sp_book_cylinder(p_user IN NUMBER, p_conn IN NUMBER, p_type IN VARCHAR2,
                                             p_booking OUT NUMBER, p_status OUT VARCHAR2) IS
  v_cnt NUMBER; v_inv NUMBER; v_wh NUMBER; v_avail NUMBER; v_price NUMBER;
BEGIN
  p_booking := NULL;
  v_price := fn_cylinder_price(p_type);
  IF v_price IS NULL THEN p_status := 'INVALID_TYPE'; RETURN; END IF;
  SELECT COUNT(*) INTO v_cnt FROM connections WHERE conn_id = p_conn AND user_id = p_user AND status = 'ACTIVE';
  IF v_cnt = 0 THEN p_status := 'INVALID_CONNECTION'; RETURN; END IF;
  SAVEPOINT sp_before_reserve;
  BEGIN
    SELECT inv_id, wh_id INTO v_inv, v_wh FROM (
      SELECT inv_id, wh_id FROM inventory WHERE cyl_type = p_type AND qty_full - qty_reserved > 0
      ORDER BY qty_full - qty_reserved DESC) WHERE ROWNUM = 1;
    SELECT qty_full - qty_reserved INTO v_avail FROM inventory WHERE inv_id = v_inv FOR UPDATE;   -- row lock: no overselling
    IF v_avail <= 0 THEN ROLLBACK TO sp_before_reserve; p_status := 'OUT_OF_STOCK'; RETURN; END IF;
    UPDATE inventory SET qty_reserved = qty_reserved + 1 WHERE inv_id = v_inv;
    p_booking := seq_bookings.NEXTVAL;
    INSERT INTO bookings(booking_id, user_id, conn_id, wh_id, inv_id, cyl_type, price, status)
    VALUES (p_booking, p_user, p_conn, v_wh, v_inv, p_type, v_price, 'BOOKED');   -- unique index blocks duplicates
    COMMIT;
    p_status := 'OK';
  EXCEPTION
    WHEN NO_DATA_FOUND THEN ROLLBACK TO sp_before_reserve; p_booking := NULL; p_status := 'OUT_OF_STOCK';
    WHEN DUP_VAL_ON_INDEX THEN ROLLBACK TO sp_before_reserve; p_booking := NULL; p_status := 'DUPLICATE_BOOKING';
  END;
END;
/
CREATE OR REPLACE PROCEDURE sp_cancel_booking(p_booking IN NUMBER, p_user IN NUMBER, p_role IN VARCHAR2, p_status OUT VARCHAR2) IS
  v_status VARCHAR2(20); v_owner NUMBER; v_inv NUMBER;
BEGIN
  SELECT status, user_id, inv_id INTO v_status, v_owner, v_inv FROM bookings WHERE booking_id = p_booking FOR UPDATE;
  IF p_role <> 'ADMIN' AND v_owner <> p_user THEN p_status := 'FORBIDDEN'; ROLLBACK; RETURN; END IF;
  IF v_status <> 'BOOKED' THEN p_status := 'NOT_CANCELLABLE'; ROLLBACK; RETURN; END IF;
  UPDATE bookings SET status = 'CANCELLED', updated_at = SYSDATE WHERE booking_id = p_booking;
  UPDATE inventory SET qty_reserved = qty_reserved - 1 WHERE inv_id = v_inv;
  COMMIT;
  p_status := 'OK';
EXCEPTION WHEN NO_DATA_FOUND THEN p_status := 'NOT_FOUND';
END;
/
CREATE OR REPLACE PROCEDURE sp_update_delivery(p_booking IN NUMBER, p_new IN VARCHAR2, p_status OUT VARCHAR2) IS
  v_cur VARCHAR2(20); v_wh NUMBER; v_inv NUMBER; v_worker NUMBER; v_veh NUMBER;
BEGIN
  SELECT status, wh_id, inv_id INTO v_cur, v_wh, v_inv FROM bookings WHERE booking_id = p_booking FOR UPDATE;
  IF NOT ((v_cur = 'BOOKED' AND p_new = 'SCHEDULED') OR (v_cur = 'SCHEDULED' AND p_new = 'OUT_FOR_DELIVERY')
          OR (v_cur = 'OUT_FOR_DELIVERY' AND p_new = 'DELIVERED')) THEN
    p_status := 'INVALID_TRANSITION'; ROLLBACK; RETURN;
  END IF;
  IF p_new = 'SCHEDULED' THEN
    BEGIN
      SELECT worker_id INTO v_worker FROM (SELECT worker_id FROM workers WHERE wh_id = v_wh AND status = 'AVAILABLE' ORDER BY worker_id) WHERE ROWNUM = 1;
      SELECT veh_id INTO v_veh FROM (SELECT veh_id FROM vehicles WHERE wh_id = v_wh AND status = 'AVAILABLE' ORDER BY veh_id) WHERE ROWNUM = 1;
    EXCEPTION WHEN NO_DATA_FOUND THEN p_status := 'NO_WORKER_OR_VEHICLE'; ROLLBACK; RETURN;
    END;
    UPDATE workers SET status = 'BUSY' WHERE worker_id = v_worker;
    UPDATE vehicles SET status = 'BUSY' WHERE veh_id = v_veh;
    INSERT INTO deliveries(delivery_id, booking_id, worker_id, vehicle_id, status, scheduled_at)
    VALUES (seq_deliveries.NEXTVAL, p_booking, v_worker, v_veh, 'SCHEDULED', SYSDATE);
  ELSE
    UPDATE deliveries SET status = p_new, delivered_at = CASE WHEN p_new = 'DELIVERED' THEN SYSDATE END WHERE booking_id = p_booking;
    IF p_new = 'DELIVERED' THEN
      UPDATE inventory SET qty_full = qty_full - 1, qty_reserved = qty_reserved - 1 WHERE inv_id = v_inv;
      UPDATE workers SET status = 'AVAILABLE' WHERE worker_id = (SELECT worker_id FROM deliveries WHERE booking_id = p_booking);
      UPDATE vehicles SET status = 'AVAILABLE' WHERE veh_id = (SELECT vehicle_id FROM deliveries WHERE booking_id = p_booking);
    END IF;
  END IF;
  UPDATE bookings SET status = p_new, updated_at = SYSDATE WHERE booking_id = p_booking;
  COMMIT;
  p_status := 'OK';
EXCEPTION WHEN NO_DATA_FOUND THEN p_status := 'NOT_FOUND';
END;
/
CREATE OR REPLACE PROCEDURE sp_redeem_points(p_user IN NUMBER, p_points IN NUMBER, p_status OUT VARCHAR2) IS
BEGIN
  IF p_points IS NULL OR p_points <= 0 THEN p_status := 'INVALID_POINTS'; RETURN; END IF;
  IF fn_points_balance(p_user) < p_points THEN p_status := 'INSUFFICIENT_POINTS'; RETURN; END IF;
  INSERT INTO rewards_txn(txn_id, user_id, points, txn_type) VALUES (seq_rewards.NEXTVAL, p_user, p_points, 'REDEEM');
  COMMIT;
  p_status := 'OK';
END;
/
CREATE OR REPLACE PROCEDURE sp_assign_technician(p_sr IN NUMBER, p_status OUT VARCHAR2) IS
  v_state VARCHAR2(10); v_city VARCHAR2(40); v_tech NUMBER;
BEGIN
  SELECT status INTO v_state FROM service_requests WHERE sr_id = p_sr FOR UPDATE;
  IF v_state <> 'OPEN' THEN p_status := 'NOT_OPEN'; ROLLBACK; RETURN; END IF;
  SELECT u.city INTO v_city FROM service_requests s JOIN users u ON u.user_id = s.user_id WHERE s.sr_id = p_sr;
  BEGIN
    SELECT tech_id INTO v_tech FROM (
      SELECT t.tech_id FROM technicians t WHERE t.available = 1 AND t.city = v_city
      ORDER BY (SELECT COUNT(*) FROM service_requests x WHERE x.tech_id = t.tech_id AND x.status = 'ASSIGNED'), t.tech_id) WHERE ROWNUM = 1;
  EXCEPTION WHEN NO_DATA_FOUND THEN
    SELECT tech_id INTO v_tech FROM (
      SELECT t.tech_id FROM technicians t WHERE t.available = 1
      ORDER BY (SELECT COUNT(*) FROM service_requests x WHERE x.tech_id = t.tech_id AND x.status = 'ASSIGNED'), t.tech_id) WHERE ROWNUM = 1;
  END;
  UPDATE service_requests SET tech_id = v_tech, status = 'ASSIGNED' WHERE sr_id = p_sr;
  COMMIT;
  p_status := 'OK';
EXCEPTION WHEN NO_DATA_FOUND THEN p_status := 'NOT_FOUND_OR_NO_TECHNICIAN';
END;
/
CREATE OR REPLACE PROCEDURE sp_complete_service(p_sr IN NUMBER, p_status OUT VARCHAR2) IS
BEGIN
  UPDATE service_requests SET status = 'DONE' WHERE sr_id = p_sr AND status IN ('OPEN','ASSIGNED');
  IF SQL%ROWCOUNT = 0 THEN p_status := 'NOT_FOUND_OR_DONE'; ROLLBACK; ELSE COMMIT; p_status := 'OK'; END IF;
END;
/
CREATE OR REPLACE PROCEDURE sp_restock(p_inv IN NUMBER, p_qty IN NUMBER, p_status OUT VARCHAR2) IS
BEGIN
  IF p_qty IS NULL OR p_qty <= 0 OR p_qty > 10000 THEN p_status := 'INVALID_QTY'; RETURN; END IF;
  UPDATE inventory SET qty_full = qty_full + p_qty WHERE inv_id = p_inv;
  IF SQL%ROWCOUNT = 0 THEN p_status := 'NOT_FOUND'; ROLLBACK; ELSE COMMIT; p_status := 'OK'; END IF;
END;
/
