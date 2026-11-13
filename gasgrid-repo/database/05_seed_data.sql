-- Sample data (all rows are SAMPLE data). Runs only when USERS is empty, so re-running never duplicates.
-- Demo/sample passwords are set by the backend on first start (pw_hash = 'SEED').
DECLARE
  v_n NUMBER;
  v_city VARCHAR2(40);
  v_u NUMBER; v_idx NUMBER; v_conn NUMBER; v_inv NUMBER; v_st VARCHAR2(20);
BEGIN
  SELECT COUNT(*) INTO v_n FROM users;
  IF v_n > 0 THEN RETURN; END IF;
  DECLARE
    CITIES CONSTANT VARCHAR2(200) := 'Dehradun,Roorkee,Delhi,Jaipur,Chandigarh,Lucknow,Mumbai,Pune,Bengaluru,Hyderabad';
    PINS   CONSTANT VARCHAR2(100) := '2480,2476,1100,3020,1600,2260,4000,4110,5600,5000';
    RTO    CONSTANT VARCHAR2(100) := 'UK07,UK08,DL01,RJ14,CH01,UP32,MH01,MH12,KA01,TS09';
    FN     CONSTANT VARCHAR2(200) := 'Aarav,Priya,Rohit,Sneha,Vikram,Anjali,Karan,Neha,Arjun,Pooja,Manish,Divya,Amit,Kavya,Rahul';
    LN     CONSTANT VARCHAR2(200) := 'Sharma,Verma,Singh,Rawat,Patel,Iyer,Reddy,Gupta,Joshi,Nair,Bisht,Khan,Mehta,Chauhan,Kapoor';
  BEGIN
    INSERT INTO cylinder_types(cyl_type, description, price, low_stock_threshold) VALUES ('14.2KG', '14.2 kg domestic cylinder', 903, 20);
    INSERT INTO cylinder_types(cyl_type, description, price, low_stock_threshold) VALUES ('19KG', '19 kg commercial cylinder', 1802, 10);
    -- users: 1 admin, 2 distributor, 3 technician, 4 demo customer, 5..104 sample customers
    INSERT INTO users(user_id, full_name, email, phone, city, pw_hash, role) VALUES (1, 'Demo Admin (sample)', 'admin@gasgrid.in', '9876500001', 'Roorkee', 'SEED', 'ADMIN');
    INSERT INTO users(user_id, full_name, email, phone, city, pw_hash, role) VALUES (2, 'Demo Distributor (sample)', 'distributor@gasgrid.in', '9876500002', 'Roorkee', 'SEED', 'DISTRIBUTOR');
    INSERT INTO users(user_id, full_name, email, phone, city, pw_hash, role) VALUES (3, 'Demo Technician (sample)', 'technician@gasgrid.in', '9876500003', 'Roorkee', 'SEED', 'TECHNICIAN');
    INSERT INTO users(user_id, full_name, email, phone, city, pw_hash, role) VALUES (4, 'Demo Customer (sample)', 'customer@gasgrid.in', '9876500004', 'Roorkee', 'SEED', 'CUSTOMER');
    FOR i IN 1..100 LOOP
      INSERT INTO users(user_id, full_name, email, phone, city, pw_hash, role)
      VALUES (4 + i, fn_pick(FN, i) || ' ' || fn_pick(LN, i * 3 + 1), 'cust' || i || '@sample.in', '9' || LPAD(MOD(i * 7919123, 1000000000), 9, '0'), fn_pick(CITIES, i), 'SEED', 'CUSTOMER');
    END LOOP;
    FOR i IN 1..15 LOOP
      INSERT INTO warehouses(wh_id, wh_name, city, capacity) VALUES (i, fn_pick(CITIES, i - 1) || ' Depot ' || i, fn_pick(CITIES, i - 1), 2000 + i * 100);
    END LOOP;
    FOR i IN 1..150 LOOP
      v_u := 4 + MOD(i - 1, 101);
      SELECT city INTO v_city FROM users WHERE user_id = v_u;
      v_idx := CASE WHEN v_u <= 4 THEN 1 ELSE MOD(v_u - 4, 10) END;
      INSERT INTO connections(conn_id, user_id, address, city, pin)
      VALUES (i, v_u, (1 + MOD(i * 13, 99)) || ', Sample Nagar, ' || v_city, v_city,
              fn_pick(PINS, v_idx) || LPAD(MOD(i * 7, 100), 2, '0'));
    END LOOP;
    FOR i IN 1..300 LOOP
      INSERT INTO inventory(inv_id, wh_id, cyl_type, batch_no, qty_full, qty_reserved)
      VALUES (i, MOD(i - 1, 15) + 1, fn_pick('14.2KG,19KG', i - 1), i, 30 + MOD(i * 17, 100), 5 + MOD(i, 10));
    END LOOP;
    FOR i IN 1..25 LOOP
      INSERT INTO vehicles(veh_id, reg_no, wh_id, capacity, status)
      VALUES (i, fn_pick(RTO, i) || ' ' || CHR(65 + MOD(i, 26)) || CHR(65 + MOD(i * 3, 26)) || ' ' || LPAD(1000 + MOD(i * 377, 9000), 4, '0'), MOD(i - 1, 15) + 1, 40 + MOD(i, 3) * 10, 'AVAILABLE');
    END LOOP;
    FOR i IN 1..30 LOOP
      INSERT INTO workers(worker_id, full_name, phone, wh_id, status)
      VALUES (i, fn_pick(FN, i + 4) || ' ' || fn_pick(LN, i + 7), '8' || LPAD(MOD(i * 6113417, 1000000000), 9, '0'), MOD(i - 1, 15) + 1, 'AVAILABLE');
    END LOOP;
    FOR i IN 1..15 LOOP
      INSERT INTO technicians(tech_id, user_id, full_name, phone, city, skill, available)
      VALUES (i, CASE WHEN i = 1 THEN 3 END, CASE WHEN i = 1 THEN 'Demo Technician (sample)' ELSE fn_pick(FN, i + 2) || ' ' || fn_pick(LN, i + 5) END,
              '7' || LPAD(MOD(i * 5231907, 1000000000), 9, '0'), fn_pick(CITIES, i - 1), fn_pick('Leak repair,Regulator,Stove fitting,Pipeline', i), 1);
    END LOOP;
    FOR i IN 1..60 LOOP
      INSERT INTO family_members(fam_id, conn_id, member_name, relation, can_book)
      VALUES (i, MOD(i * 7, 150) + 1, fn_pick(FN, i + 9) || ' ' || fn_pick(LN, i), fn_pick('Spouse,Parent,Child,Sibling', i), MOD(i, 2));
    END LOOP;
    FOR i IN 1..200 LOOP
      v_conn := MOD(i - 1, 150) + 1;
      v_inv := (MOD(i - 1, 15) + 1) + 15 * MOD(i, 20);
      IF i <= 150 THEN v_st := fn_pick('BOOKED,SCHEDULED,OUT_FOR_DELIVERY,DELIVERED', i); ELSE v_st := fn_pick('DELIVERED,CANCELLED', i); END IF;
      INSERT INTO bookings(booking_id, user_id, conn_id, wh_id, inv_id, cyl_type, price, status, booked_at)
      VALUES (i, 4 + MOD(v_conn - 1, 101), v_conn, MOD(i - 1, 15) + 1, v_inv, fn_pick('14.2KG,19KG', v_inv - 1),
              CASE fn_pick('14.2KG,19KG', v_inv - 1) WHEN '14.2KG' THEN 903 ELSE 1802 END, v_st, SYSDATE - MOD(i * 7, 90));
    END LOOP;
    FOR r IN (SELECT booking_id, status, booked_at FROM (SELECT * FROM bookings WHERE status IN ('SCHEDULED','OUT_FOR_DELIVERY','DELIVERED') ORDER BY booking_id) WHERE ROWNUM <= 100) LOOP
      INSERT INTO deliveries(delivery_id, booking_id, worker_id, vehicle_id, status, scheduled_at, delivered_at)
      VALUES (r.booking_id, r.booking_id, MOD(r.booking_id, 30) + 1, MOD(r.booking_id, 25) + 1, r.status, r.booked_at + 1, CASE WHEN r.status = 'DELIVERED' THEN r.booked_at + 2 END);
    END LOOP;
    FOR i IN 1..50 LOOP
      INSERT INTO service_requests(sr_id, user_id, tech_id, issue, priority, status, created_at)
      VALUES (i, 4 + MOD(i * 7, 101), CASE WHEN MOD(i, 3) = 0 THEN NULL ELSE MOD(i, 15) + 1 END, fn_pick('Gas leak check,Regulator replacement,Stove fitting,Pipe replacement,Hose change', i),
              1 + MOD(i, 3), CASE WHEN MOD(i, 3) = 0 THEN 'OPEN' WHEN MOD(i, 3) = 1 THEN 'ASSIGNED' ELSE 'DONE' END, SYSDATE - MOD(i, 30));
    END LOOP;
    FOR i IN 1..100 LOOP
      INSERT INTO rewards_txn(txn_id, user_id, points, txn_type, created_at)
      VALUES (i, 4 + MOD(i - 1, 50), CASE WHEN i > 50 AND MOD(i, 5) = 0 THEN 20 ELSE 50 + MOD(i, 5) * 10 END,
              CASE WHEN i > 50 AND MOD(i, 5) = 0 THEN 'REDEEM' ELSE 'EARN' END, SYSDATE - MOD(i, 60));
    END LOOP;
    FOR i IN 1..50 LOOP
      INSERT INTO os_simulations(sim_id, user_id, module, algorithm, input_text, summary, is_sample, created_at)
      VALUES (i, 1, fn_pick('CPU,SYNC,DEADLOCK,MEMORY,DISK,PROCESS', i), fn_pick('FCFS,SJF,SRTF,RR,PRIORITY,MLFQ', i), 'sample input', 'Sample simulation record #' || i, 1, SYSDATE - MOD(i, 20));
    END LOOP;
    FOR i IN 1..100 LOOP
      INSERT INTO os_processes(proc_id, sim_id, pid, job_type, state, arrival, burst, priority, resource_need)
      VALUES (i, MOD(i - 1, 50) + 1, i, fn_pick('BOOKING,SERVICE,REPORT', i), fn_pick('NEW,READY,RUNNING,WAITING,TERMINATED', i), MOD(i, 10), 1 + MOD(i * 3, 9), 1 + MOD(i, 3), fn_pick('STOCK,TECHNICIAN,EVENT', i));
    END LOOP;
    COMMIT;
  END;
END;
/
