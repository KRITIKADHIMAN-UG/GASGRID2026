-- Stored procedure to update order status
CREATE OR REPLACE PROCEDURE update_order_status(p_order_id IN NUMBER, p_status IN VARCHAR2) AS
BEGIN
  IF p_status NOT IN ('Pending','Scheduled','Out for Delivery','Delivered','Cancelled') THEN
    RAISE_APPLICATION_ERROR(-20001,'Invalid order status');
  END IF;
  UPDATE orders SET status=p_status WHERE order_id=p_order_id;
  IF SQL%ROWCOUNT=0 THEN RAISE_APPLICATION_ERROR(-20002,'Order not found'); END IF;
END;
/
CREATE OR REPLACE FUNCTION get_order_total(p_order_id IN NUMBER) RETURN NUMBER AS v_total NUMBER;
BEGIN SELECT total_amount INTO v_total FROM orders WHERE order_id=p_order_id; RETURN v_total;
EXCEPTION WHEN NO_DATA_FOUND THEN RETURN NULL; END;
/
