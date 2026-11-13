-- GasGrid schema (Oracle 11g XE compatible). Run as user GASGRID.
-- All sequences start high so explicit sample-data ids never collide.
CREATE SEQUENCE seq_users START WITH 10000;
CREATE SEQUENCE seq_connections START WITH 10000;
CREATE SEQUENCE seq_family START WITH 10000;
CREATE SEQUENCE seq_warehouses START WITH 10000;
CREATE SEQUENCE seq_inventory START WITH 10000;
CREATE SEQUENCE seq_vehicles START WITH 10000;
CREATE SEQUENCE seq_workers START WITH 10000;
CREATE SEQUENCE seq_technicians START WITH 10000;
CREATE SEQUENCE seq_bookings START WITH 10000;
CREATE SEQUENCE seq_deliveries START WITH 10000;
CREATE SEQUENCE seq_service START WITH 10000;
CREATE SEQUENCE seq_rewards START WITH 10000;
CREATE SEQUENCE seq_notifications START WITH 10000;
CREATE SEQUENCE seq_os_sim START WITH 10000;
CREATE SEQUENCE seq_os_proc START WITH 10000;
CREATE SEQUENCE seq_audit START WITH 10000;

CREATE TABLE users (
  user_id    NUMBER PRIMARY KEY,
  full_name  VARCHAR2(80)  NOT NULL,
  email      VARCHAR2(120) NOT NULL UNIQUE,
  phone      VARCHAR2(15),
  city       VARCHAR2(40),
  pw_hash    VARCHAR2(200) NOT NULL,
  role       VARCHAR2(12)  NOT NULL CHECK (role IN ('CUSTOMER','DISTRIBUTOR','TECHNICIAN','ADMIN')),
  is_sample  NUMBER(1) DEFAULT 1 NOT NULL,
  created_at DATE DEFAULT SYSDATE NOT NULL
);
CREATE TABLE cylinder_types (
  cyl_type            VARCHAR2(10) PRIMARY KEY,
  description         VARCHAR2(60) NOT NULL,
  price               NUMBER(8,2) NOT NULL CHECK (price > 0),
  low_stock_threshold NUMBER NOT NULL
);
CREATE TABLE warehouses (
  wh_id    NUMBER PRIMARY KEY,
  wh_name  VARCHAR2(80) NOT NULL UNIQUE,
  city     VARCHAR2(40) NOT NULL,
  capacity NUMBER NOT NULL CHECK (capacity > 0)
);
CREATE TABLE connections (
  conn_id  NUMBER PRIMARY KEY,
  user_id  NUMBER NOT NULL REFERENCES users(user_id),
  address  VARCHAR2(200) NOT NULL,
  city     VARCHAR2(40),
  pin      CHAR(6) NOT NULL CHECK (REGEXP_LIKE(pin, '^[0-9]{6}$')),
  status   VARCHAR2(10) DEFAULT 'ACTIVE' NOT NULL CHECK (status IN ('ACTIVE','SUSPENDED','CLOSED'))
);
CREATE TABLE family_members (
  fam_id   NUMBER PRIMARY KEY,
  conn_id  NUMBER NOT NULL REFERENCES connections(conn_id),
  member_name VARCHAR2(80) NOT NULL,
  relation VARCHAR2(20) NOT NULL,
  can_book NUMBER(1) DEFAULT 0 NOT NULL CHECK (can_book IN (0,1))
);
CREATE TABLE inventory (
  inv_id       NUMBER PRIMARY KEY,
  wh_id        NUMBER NOT NULL REFERENCES warehouses(wh_id),
  cyl_type     VARCHAR2(10) NOT NULL REFERENCES cylinder_types(cyl_type),
  batch_no     NUMBER NOT NULL,
  qty_full     NUMBER DEFAULT 0 NOT NULL CHECK (qty_full >= 0),
  qty_reserved NUMBER DEFAULT 0 NOT NULL CHECK (qty_reserved >= 0),
  CONSTRAINT uq_inventory_batch UNIQUE (wh_id, cyl_type, batch_no)
);
CREATE TABLE vehicles (
  veh_id   NUMBER PRIMARY KEY,
  reg_no   VARCHAR2(15) NOT NULL UNIQUE,
  wh_id    NUMBER NOT NULL REFERENCES warehouses(wh_id),
  capacity NUMBER NOT NULL,
  status   VARCHAR2(10) DEFAULT 'AVAILABLE' NOT NULL CHECK (status IN ('AVAILABLE','BUSY','SERVICE'))
);
CREATE TABLE workers (
  worker_id NUMBER PRIMARY KEY,
  full_name VARCHAR2(80) NOT NULL,
  phone     VARCHAR2(15),
  wh_id     NUMBER NOT NULL REFERENCES warehouses(wh_id),
  status    VARCHAR2(10) DEFAULT 'AVAILABLE' NOT NULL CHECK (status IN ('AVAILABLE','BUSY','OFF'))
);
CREATE TABLE technicians (
  tech_id   NUMBER PRIMARY KEY,
  user_id   NUMBER REFERENCES users(user_id),
  full_name VARCHAR2(80) NOT NULL,
  phone     VARCHAR2(15),
  city      VARCHAR2(40) NOT NULL,
  skill     VARCHAR2(30),
  available NUMBER(1) DEFAULT 1 NOT NULL CHECK (available IN (0,1))
);
CREATE TABLE bookings (
  booking_id NUMBER PRIMARY KEY,
  user_id    NUMBER NOT NULL REFERENCES users(user_id),
  conn_id    NUMBER NOT NULL REFERENCES connections(conn_id),
  wh_id      NUMBER NOT NULL REFERENCES warehouses(wh_id),
  inv_id     NUMBER NOT NULL REFERENCES inventory(inv_id),
  cyl_type   VARCHAR2(10) NOT NULL REFERENCES cylinder_types(cyl_type),
  price      NUMBER(8,2) NOT NULL CHECK (price > 0),
  status     VARCHAR2(20) DEFAULT 'BOOKED' NOT NULL CHECK (status IN ('BOOKED','SCHEDULED','OUT_FOR_DELIVERY','DELIVERED','CANCELLED')),
  booked_at  DATE DEFAULT SYSDATE NOT NULL,
  updated_at DATE
);
CREATE TABLE deliveries (
  delivery_id  NUMBER PRIMARY KEY,
  booking_id   NUMBER NOT NULL UNIQUE REFERENCES bookings(booking_id),
  worker_id    NUMBER NOT NULL REFERENCES workers(worker_id),
  vehicle_id   NUMBER NOT NULL REFERENCES vehicles(veh_id),
  status       VARCHAR2(20) NOT NULL,
  scheduled_at DATE,
  delivered_at DATE
);
CREATE TABLE service_requests (
  sr_id      NUMBER PRIMARY KEY,
  user_id    NUMBER NOT NULL REFERENCES users(user_id),
  tech_id    NUMBER REFERENCES technicians(tech_id),
  issue      VARCHAR2(200) NOT NULL,
  priority   NUMBER(1) DEFAULT 2 NOT NULL CHECK (priority BETWEEN 1 AND 3),
  status     VARCHAR2(10) DEFAULT 'OPEN' NOT NULL CHECK (status IN ('OPEN','ASSIGNED','DONE')),
  created_at DATE DEFAULT SYSDATE NOT NULL
);
CREATE TABLE rewards_txn (
  txn_id     NUMBER PRIMARY KEY,
  user_id    NUMBER NOT NULL REFERENCES users(user_id),
  points     NUMBER NOT NULL CHECK (points > 0),
  txn_type   VARCHAR2(6) NOT NULL CHECK (txn_type IN ('EARN','REDEEM')),
  booking_id NUMBER REFERENCES bookings(booking_id),
  created_at DATE DEFAULT SYSDATE NOT NULL
);
CREATE TABLE notifications (
  notif_id   NUMBER PRIMARY KEY,
  user_id    NUMBER NOT NULL REFERENCES users(user_id),
  message    VARCHAR2(300) NOT NULL,
  is_read    NUMBER(1) DEFAULT 0 NOT NULL,
  created_at DATE DEFAULT SYSDATE NOT NULL
);
CREATE TABLE os_simulations (
  sim_id     NUMBER PRIMARY KEY,
  user_id    NUMBER NOT NULL REFERENCES users(user_id),
  module     VARCHAR2(20) NOT NULL,
  algorithm  VARCHAR2(30),
  input_text VARCHAR2(2000),
  summary    VARCHAR2(500),
  is_sample  NUMBER(1) DEFAULT 0 NOT NULL,
  created_at DATE DEFAULT SYSDATE NOT NULL
);
CREATE TABLE os_processes (
  proc_id       NUMBER PRIMARY KEY,
  sim_id        NUMBER REFERENCES os_simulations(sim_id),
  pid           NUMBER NOT NULL,
  job_type      VARCHAR2(10) NOT NULL,
  state         VARCHAR2(12) NOT NULL,
  arrival       NUMBER NOT NULL,
  burst         NUMBER NOT NULL,
  priority      NUMBER NOT NULL,
  resource_need VARCHAR2(12)
);
CREATE TABLE audit_logs (
  log_id    NUMBER PRIMARY KEY,
  user_id   NUMBER,
  action    VARCHAR2(40) NOT NULL,
  details   VARCHAR2(300),
  logged_at DATE DEFAULT SYSDATE NOT NULL
);
CREATE TABLE password_resets (
  token      VARCHAR2(64) PRIMARY KEY,
  user_id    NUMBER NOT NULL REFERENCES users(user_id),
  expires_at DATE NOT NULL,
  used       NUMBER(1) DEFAULT 0 NOT NULL
);
