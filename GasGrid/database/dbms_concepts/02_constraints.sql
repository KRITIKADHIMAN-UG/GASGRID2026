-- Constraints and relationships
ALTER TABLE users ADD CONSTRAINT uq_users_phone UNIQUE(phone);
ALTER TABLE users ADD CONSTRAINT ck_users_role CHECK(role IN ('user','admin'));
ALTER TABLE users ADD CONSTRAINT ck_users_status CHECK(account_status IN ('Active','Inactive','Suspended'));
ALTER TABLE orders ADD CONSTRAINT fk_orders_user FOREIGN KEY(user_id) REFERENCES users(user_id);
ALTER TABLE orders ADD CONSTRAINT ck_order_type CHECK(cylinder_type IN ('Domestic','Commercial'));
ALTER TABLE orders ADD CONSTRAINT ck_order_quantity CHECK(quantity BETWEEN 1 AND 5);
ALTER TABLE orders ADD CONSTRAINT ck_order_priority CHECK(priority BETWEEN 1 AND 3);
ALTER TABLE orders ADD CONSTRAINT ck_order_status CHECK(status IN ('Pending','Scheduled','Out for Delivery','Delivered','Cancelled'));
ALTER TABLE complaints ADD CONSTRAINT fk_complaints_user FOREIGN KEY(user_id) REFERENCES users(user_id);
ALTER TABLE process_logs ADD CONSTRAINT fk_logs_order FOREIGN KEY(order_id) REFERENCES orders(order_id);
ALTER TABLE inventory ADD CONSTRAINT ck_inventory_available CHECK(available >= 0);
