-- Transaction demonstration: reserve stock and create order as one unit of work
UPDATE inventory SET available=available-1 WHERE item_name='Domestic LPG Cylinders' AND available>=1;
-- Check SQL%ROWCOUNT in PL/SQL before inserting an order in a real transaction.
INSERT INTO orders(order_id,user_id,cylinder_type,quantity,unit_price,total_amount,priority,status,delivery_address) VALUES(orders_seq.NEXTVAL,2,'Domestic',1,950,950,2,'Pending','Clement Town, Dehradun');
-- Choose one: COMMIT; or ROLLBACK;
ROLLBACK;
