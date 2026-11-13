-- Indexes
CREATE INDEX ix_bookings_user   ON bookings(user_id);
CREATE INDEX ix_bookings_status ON bookings(status);
CREATE INDEX ix_bookings_wh     ON bookings(wh_id);
CREATE INDEX ix_conn_user       ON connections(user_id);
CREATE INDEX ix_inventory_type  ON inventory(cyl_type, wh_id);
CREATE INDEX ix_service_queue   ON service_requests(status, priority);
CREATE INDEX ix_audit_time      ON audit_logs(logged_at);
CREATE INDEX ix_rewards_user    ON rewards_txn(user_id);
-- Duplicate-booking guard: at most ONE active booking per connection (function-based unique index)
CREATE UNIQUE INDEX uq_active_booking ON bookings (CASE WHEN status IN ('BOOKED','SCHEDULED','OUT_FOR_DELIVERY') THEN conn_id END);

-- Views
CREATE OR REPLACE VIEW vw_booking_details AS
SELECT b.booking_id, b.user_id, u.full_name AS customer, u.email, c.address, w.city AS wh_city,
       b.cyl_type, b.price, b.status, b.booked_at,
       d.status AS delivery_status, wk.full_name AS worker_name, v.reg_no AS vehicle_reg
FROM bookings b
JOIN users u        ON u.user_id = b.user_id
JOIN connections c  ON c.conn_id = b.conn_id
JOIN warehouses w   ON w.wh_id = b.wh_id
LEFT JOIN deliveries d ON d.booking_id = b.booking_id
LEFT JOIN workers wk   ON wk.worker_id = d.worker_id
LEFT JOIN vehicles v   ON v.veh_id = d.vehicle_id;

CREATE OR REPLACE VIEW vw_city_bookings AS
SELECT w.city, COUNT(*) AS total,
       SUM(CASE WHEN b.status = 'DELIVERED' THEN 1 ELSE 0 END) AS delivered,
       SUM(CASE WHEN b.status = 'CANCELLED' THEN 1 ELSE 0 END) AS cancelled,
       NVL(SUM(CASE WHEN b.status = 'DELIVERED' THEN b.price END), 0) AS revenue
FROM bookings b JOIN warehouses w ON w.wh_id = b.wh_id
GROUP BY w.city;

CREATE OR REPLACE VIEW vw_inventory_status AS
SELECT w.wh_id, w.wh_name, w.city, SUM(i.qty_full) AS qty_full, SUM(i.qty_reserved) AS qty_reserved,
       SUM(i.qty_full - i.qty_reserved) AS available
FROM warehouses w JOIN inventory i ON i.wh_id = w.wh_id
GROUP BY w.wh_id, w.wh_name, w.city;

CREATE OR REPLACE VIEW vw_service_queue AS
SELECT s.sr_id, s.user_id, u.full_name AS customer, u.city, s.issue, s.priority, s.status, s.created_at,
       t.full_name AS technician
FROM service_requests s JOIN users u ON u.user_id = s.user_id
LEFT JOIN technicians t ON t.tech_id = s.tech_id;

CREATE OR REPLACE VIEW vw_customer_summary AS
SELECT u.user_id, u.full_name, u.city,
       (SELECT COUNT(*) FROM bookings b WHERE b.user_id = u.user_id) AS total_bookings,
       (SELECT COUNT(*) FROM bookings b WHERE b.user_id = u.user_id AND b.status = 'DELIVERED') AS delivered,
       (SELECT NVL(SUM(CASE WHEN r.txn_type = 'EARN' THEN r.points ELSE -r.points END), 0) FROM rewards_txn r WHERE r.user_id = u.user_id) AS points_balance
FROM users u WHERE u.role = 'CUSTOMER';

CREATE OR REPLACE VIEW vw_inventory_by_type AS
SELECT t.cyl_type, t.description, t.price, NVL(SUM(i.qty_full), 0) AS qty_full, NVL(SUM(i.qty_reserved), 0) AS qty_reserved,
       NVL(SUM(i.qty_full - i.qty_reserved), 0) AS available
FROM cylinder_types t LEFT JOIN inventory i ON i.cyl_type = t.cyl_type
GROUP BY t.cyl_type, t.description, t.price;

CREATE OR REPLACE VIEW vw_type_bookings AS
SELECT b.cyl_type, COUNT(*) AS total,
       SUM(CASE WHEN b.status = 'DELIVERED' THEN 1 ELSE 0 END) AS delivered,
       NVL(SUM(CASE WHEN b.status = 'DELIVERED' THEN b.price END), 0) AS revenue
FROM bookings b GROUP BY b.cyl_type;
