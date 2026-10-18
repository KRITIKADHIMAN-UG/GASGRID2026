-- CRUD examples
-- CREATE
INSERT INTO complaints(complaint_id,user_id,category,details,status) VALUES(1,2,'Late Delivery','Demo complaint for SQL practice','Open');
-- READ
SELECT order_id,cylinder_type,quantity,total_amount,status FROM orders WHERE user_id=2 ORDER BY created_at DESC;
-- UPDATE
UPDATE orders SET status='Scheduled' WHERE order_id=1 AND status='Pending';
-- DELETE (use carefully in a practice database)
DELETE FROM complaints WHERE complaint_id=1;
ROLLBACK; -- demonstrate transaction rollback of changes above
