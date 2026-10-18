-- Views and indexes
CREATE OR REPLACE VIEW pending_orders_view AS SELECT order_id,user_id,cylinder_type,quantity,total_amount,priority,created_at FROM orders WHERE status IN ('Pending','Scheduled');
CREATE INDEX idx_orders_user_status ON orders(user_id,status);
CREATE INDEX idx_complaints_status ON complaints(status);
SELECT * FROM pending_orders_view;
