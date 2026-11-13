-- COMMIT / ROLLBACK / SAVEPOINT demonstration (run manually in SQL*Plus)
UPDATE inventory SET qty_full = qty_full + 10 WHERE inv_id = 1;
SAVEPOINT after_restock;
UPDATE inventory SET qty_full = qty_full - 5 WHERE inv_id = 2;
SELECT inv_id, qty_full FROM inventory WHERE inv_id IN (1, 2);
ROLLBACK TO after_restock;      -- undoes only the second update
SELECT inv_id, qty_full FROM inventory WHERE inv_id IN (1, 2);
COMMIT;                         -- keeps the first update
-- Stored procedure with built-in savepoint (see sp_book_cylinder):
-- DECLARE b NUMBER; s VARCHAR2(30); BEGIN sp_book_cylinder(4, 1, '14.2KG', b, s); DBMS_OUTPUT.PUT_LINE(s); END;
