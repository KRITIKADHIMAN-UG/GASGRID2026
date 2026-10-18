-- Trigger to prevent negative inventory and maintain a timestamp example
CREATE OR REPLACE TRIGGER trg_inventory_nonnegative
BEFORE INSERT OR UPDATE OF available ON inventory
FOR EACH ROW
BEGIN
  IF :NEW.available < 0 THEN RAISE_APPLICATION_ERROR(-20010,'Inventory cannot be negative'); END IF;
END;
/
