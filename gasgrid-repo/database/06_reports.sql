-- Report / demonstration queries (joins, subqueries, aggregates, GROUP BY, HAVING, UNION, views)
-- 1. INNER JOIN across four tables
SELECT b.booking_id, u.full_name, w.city, b.cyl_type, b.price, b.status
FROM bookings b JOIN users u ON u.user_id = b.user_id JOIN warehouses w ON w.wh_id = b.wh_id
WHERE b.status = 'DELIVERED' ORDER BY b.booking_id;
-- 2. LEFT JOIN: customers with no booking
SELECT u.user_id, u.full_name FROM users u LEFT JOIN bookings b ON b.user_id = u.user_id
WHERE u.role = 'CUSTOMER' AND b.booking_id IS NULL;
-- 3. Aggregates + GROUP BY: revenue per city
SELECT w.city, COUNT(*) AS bookings, SUM(b.price) AS revenue, ROUND(AVG(b.price), 2) AS avg_price
FROM bookings b JOIN warehouses w ON w.wh_id = b.wh_id WHERE b.status = 'DELIVERED' GROUP BY w.city ORDER BY revenue DESC;
-- 4. HAVING: busy cities only
SELECT w.city, COUNT(*) AS bookings FROM bookings b JOIN warehouses w ON w.wh_id = b.wh_id GROUP BY w.city HAVING COUNT(*) > 15 ORDER BY bookings DESC;
-- 5. Subquery: customers with above-average bookings
SELECT user_id, total_bookings FROM vw_customer_summary WHERE total_bookings > (SELECT AVG(total_bookings) FROM vw_customer_summary);
-- 6. Correlated subquery: warehouses with low available stock
SELECT w.wh_id, w.wh_name, (SELECT SUM(i.qty_full - i.qty_reserved) FROM inventory i WHERE i.wh_id = w.wh_id) AS available
FROM warehouses w WHERE (SELECT SUM(i.qty_full - i.qty_reserved) FROM inventory i WHERE i.wh_id = w.wh_id) < 1500;
-- 7. UNION: one activity feed from bookings and service requests
SELECT 'BOOKING' AS kind, booking_id AS ref_id, booked_at AS happened FROM bookings
UNION
SELECT 'SERVICE', sr_id, created_at FROM service_requests ORDER BY happened DESC;
-- 8. Top 5 customers by points (ROWNUM pagination style, 11g compatible)
SELECT * FROM (SELECT user_id, full_name, points_balance FROM vw_customer_summary ORDER BY points_balance DESC) WHERE ROWNUM <= 5;
-- 9. Views
SELECT * FROM vw_city_bookings ORDER BY total DESC;
SELECT * FROM vw_inventory_status ORDER BY available;
-- 10. Function call
SELECT user_id, fn_points_balance(user_id) AS points FROM users WHERE user_id BETWEEN 4 AND 10;
-- 11. Separate pricing, stock and revenue per cylinder type
SELECT * FROM vw_inventory_by_type;
SELECT * FROM vw_type_bookings ORDER BY cyl_type;
