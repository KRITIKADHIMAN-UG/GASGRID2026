CREATE OR REPLACE TRIGGER trg_bookings_status_guard
BEFORE UPDATE OF status ON bookings FOR EACH ROW
BEGIN
  IF :OLD.status <> :NEW.status AND NOT (
       (:OLD.status = 'BOOKED' AND :NEW.status IN ('SCHEDULED','CANCELLED'))
    OR (:OLD.status = 'SCHEDULED' AND :NEW.status IN ('OUT_FOR_DELIVERY','CANCELLED'))
    OR (:OLD.status = 'OUT_FOR_DELIVERY' AND :NEW.status = 'DELIVERED')) THEN
    RAISE_APPLICATION_ERROR(-20001, 'Invalid booking status transition ' || :OLD.status || ' -> ' || :NEW.status);
  END IF;
END;
/
CREATE OR REPLACE TRIGGER trg_bookings_audit
AFTER INSERT OR UPDATE OF status ON bookings FOR EACH ROW
BEGIN
  INSERT INTO audit_logs(log_id, user_id, action, details)
  VALUES (seq_audit.NEXTVAL, :NEW.user_id, CASE WHEN INSERTING THEN 'BOOKING_CREATED' ELSE 'BOOKING_' || :NEW.status END, 'booking ' || :NEW.booking_id);
  INSERT INTO notifications(notif_id, user_id, message)
  VALUES (seq_notifications.NEXTVAL, :NEW.user_id, 'Booking #' || :NEW.booking_id || ' is now ' || :NEW.status);
END;
/
CREATE OR REPLACE TRIGGER trg_booking_rewards
AFTER UPDATE OF status ON bookings FOR EACH ROW
WHEN (NEW.status = 'DELIVERED' AND OLD.status <> 'DELIVERED')
BEGIN
  INSERT INTO rewards_txn(txn_id, user_id, points, txn_type, booking_id)
  VALUES (seq_rewards.NEXTVAL, :NEW.user_id, 10, 'EARN', :NEW.booking_id);
END;
/
CREATE OR REPLACE TRIGGER trg_inventory_low_stock
AFTER UPDATE OF qty_full, qty_reserved ON inventory FOR EACH ROW
DECLARE
  v_limit NUMBER;
BEGIN
  SELECT low_stock_threshold INTO v_limit FROM cylinder_types WHERE cyl_type = :NEW.cyl_type;   -- per-type threshold
  IF :NEW.qty_full - :NEW.qty_reserved < v_limit THEN
    INSERT INTO notifications(notif_id, user_id, message)
    SELECT seq_notifications.NEXTVAL, user_id, 'Low stock: batch ' || :NEW.inv_id || ' (' || :NEW.cyl_type || ')' FROM users WHERE role IN ('ADMIN','DISTRIBUTOR');
  END IF;
END;
/
CREATE OR REPLACE TRIGGER trg_users_audit
AFTER INSERT ON users FOR EACH ROW
BEGIN
  INSERT INTO audit_logs(log_id, user_id, action, details) VALUES (seq_audit.NEXTVAL, :NEW.user_id, 'USER_CREATED', :NEW.role);
END;
/
