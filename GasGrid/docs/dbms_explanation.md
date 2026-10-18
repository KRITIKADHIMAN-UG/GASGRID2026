# DBMS Concepts in GasGrid

- **Tables:** `users`, `orders`, `complaints`, `inventory`, and `process_logs` store structured data.
- **Constraints:** primary keys, foreign keys, unique constraints, check constraints, and NOT NULL protect data quality.
- **CRUD:** insert, select, update, and delete examples are in `04_crud_operations.sql`.
- **Joins:** combine order/complaint rows with customer details.
- **Views and indexes:** a view exposes pending orders; indexes help common filters.
- **Sequences:** generate numeric identifiers in Oracle.
- **Transactions:** COMMIT makes a unit of work permanent; ROLLBACK reverses uncommitted changes.
- **Procedures/functions:** reusable server-side PL/SQL logic.
- **Triggers:** enforce a rule automatically when inventory is inserted or updated.

The Oracle scripts are educational and separate from the currently supplied SQLite Flask runtime.
