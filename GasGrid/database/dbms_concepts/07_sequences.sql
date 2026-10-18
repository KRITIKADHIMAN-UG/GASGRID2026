-- Oracle sequences for primary keys
CREATE SEQUENCE users_seq START WITH 100 INCREMENT BY 1 NOCACHE;
CREATE SEQUENCE orders_seq START WITH 100 INCREMENT BY 1 NOCACHE;
CREATE SEQUENCE complaints_seq START WITH 100 INCREMENT BY 1 NOCACHE;
CREATE SEQUENCE inventory_seq START WITH 100 INCREMENT BY 1 NOCACHE;
CREATE SEQUENCE process_logs_seq START WITH 100 INCREMENT BY 1 NOCACHE;
-- Example: INSERT INTO inventory(item_id,item_name,category,available,unit) VALUES(inventory_seq.NEXTVAL,'Vehicle Unit 01','Vehicle',1,'vehicles');
