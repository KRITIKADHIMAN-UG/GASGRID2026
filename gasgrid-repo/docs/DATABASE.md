# Database (Oracle 11g+)

Tables (3NF): CYLINDER_TYPES (14.2KG domestic, 19KG commercial: separate price and low-stock threshold), USERS, CONNECTIONS, FAMILY_MEMBERS, WAREHOUSES, INVENTORY, VEHICLES, WORKERS, TECHNICIANS, BOOKINGS, DELIVERIES, SERVICE_REQUESTS, REWARDS_TXN, NOTIFICATIONS, OS_SIMULATIONS, OS_PROCESSES, AUDIT_LOGS, PASSWORD_RESETS. Every table has a PK from a sequence; foreign keys, CHECK and UNIQUE constraints enforce the rules.

Relationships: USERS 1-N CONNECTIONS 1-N FAMILY_MEMBERS; WAREHOUSES 1-N INVENTORY/VEHICLES/WORKERS; BOOKINGS N-1 USERS, CONNECTIONS, WAREHOUSES, INVENTORY; BOOKINGS 1-1 DELIVERIES (worker + vehicle); USERS 1-N SERVICE_REQUESTS N-1 TECHNICIANS; OS_SIMULATIONS 1-N OS_PROCESSES.

| DBMS topic | Where |
|---|---|
| Joins, subqueries, aggregates, GROUP BY, HAVING, UNION | `06_reports.sql`; views in `02_indexes_views.sql`; `ReportDao.java` |
| Views | VW_BOOKING_DETAILS, VW_CITY_BOOKINGS, VW_INVENTORY_STATUS, VW_SERVICE_QUEUE, VW_CUSTOMER_SUMMARY |
| Indexes | `02_indexes_views.sql`; function-based UNIQUE index UQ_ACTIVE_BOOKING = one active booking per connection |
| Transactions, COMMIT, ROLLBACK, SAVEPOINT | `sp_book_cylinder` (savepoint + row lock `FOR UPDATE` prevents overselling), `07_transactions_demo.sql` |
| Stored procedures / functions | `03_procedures_functions.sql`: book, cancel, update delivery, redeem, assign technician, complete, restock; FN_POINTS_BALANCE, FN_CYLINDER_PRICE |
| Triggers / audit logging | `04_triggers.sql`: status-transition guard, booking audit + notification, delivery rewards, low-stock alert, user audit |
| Two cylinder types | INVENTORY and BOOKINGS reference CYLINDER_TYPES; stock is reserved per type; VW_INVENTORY_BY_TYPE, VW_TYPE_BOOKINGS; SP_SET_PRICE |
| Seed once | `05_seed_data.sql` exits if USERS is not empty |
