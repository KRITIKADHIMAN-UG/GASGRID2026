-- Sample rows. Run after 01 and 07. Use non-production demo data only.
INSERT INTO users(user_id,name,phone,password_hash,role,address) VALUES(1,'GasGrid Admin','9999999999','DEMO_HASH_REPLACE_BEFORE_USE','admin','GasGrid Office, Dehradun');
INSERT INTO users(user_id,name,phone,password_hash,role,address) VALUES(2,'Demo Customer','9000000000','DEMO_HASH_REPLACE_BEFORE_USE','user','Clement Town, Dehradun');
INSERT INTO inventory(item_id,item_name,category,available,unit) VALUES(1,'Domestic LPG Cylinders','Cylinder',100,'cylinders');
INSERT INTO inventory(item_id,item_name,category,available,unit) VALUES(2,'Commercial LPG Cylinders','Cylinder',40,'cylinders');
INSERT INTO orders(order_id,user_id,cylinder_type,quantity,unit_price,total_amount,priority,status,delivery_address) VALUES(1,2,'Domestic',1,950,950,2,'Pending','Clement Town, Dehradun');
COMMIT;
