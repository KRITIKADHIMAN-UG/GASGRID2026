-- INNER JOIN: orders with customer details
SELECT o.order_id,u.name,u.phone,o.cylinder_type,o.quantity,o.total_amount,o.status FROM orders o INNER JOIN users u ON u.user_id=o.user_id;
-- LEFT JOIN: all customers, including those without orders
SELECT u.user_id,u.name,o.order_id,o.status FROM users u LEFT JOIN orders o ON o.user_id=u.user_id;
-- JOIN complaints with their customer
SELECT c.complaint_id,u.name,c.category,c.status FROM complaints c JOIN users u ON u.user_id=c.user_id;
