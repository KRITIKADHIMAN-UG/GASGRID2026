"""GasGrid academic LPG management prototype.

The Flask API owns validation, pricing, database persistence, inventory updates,

account recovery, and the OS scheduling simulation used by the admin dashboard.

SQLite is intentionally used in this version so the project runs locally without

an external database server.

"""

from flask import Flask, render_template, request, jsonify, session, redirect, url_for

import sqlite3, pathlib, re, os, subprocess, secrets

from datetime import datetime, timedelta

from functools import wraps

from werkzeug.security import generate_password_hash, check_password_hash

ROOT = pathlib.Path(__file__).resolve().parent

DB_PATH = ROOT / "gasgrid.db"

app = Flask(__name__, template_folder="../frontend", static_folder="../frontend")

app.secret_key = os.environ.get("GASGRID_SECRET", "change-this-secret-for-real-deployment")

app.config["SESSION_COOKIE_HTTPONLY"] = True

app.config["SESSION_COOKIE_SAMESITE"] = "Lax"

# Open a short-lived SQLite connection; foreign keys are enforced on every connection.

def connect_db():

    conn = sqlite3.connect(DB_PATH)

    conn.row_factory = sqlite3.Row

    conn.execute("PRAGMA foreign_keys = ON")

    return conn

def now():

    return datetime.now().isoformat(timespec="seconds")

# Create schema and idempotently seed demo accounts plus operational inventory.

def init_db():

    with connect_db() as conn:

        conn.executescript("""

        CREATE TABLE IF NOT EXISTS users (

            user_id INTEGER PRIMARY KEY AUTOINCREMENT,

            name TEXT NOT NULL,

            phone TEXT NOT NULL UNIQUE,

            password TEXT NOT NULL,

            role TEXT NOT NULL CHECK(role IN ('user','admin')),

            address TEXT NOT NULL,

            account_status TEXT NOT NULL DEFAULT 'Active' CHECK(account_status IN ('Active','Inactive','Suspended')),

            created_at TEXT NOT NULL

        );

        CREATE TABLE IF NOT EXISTS orders (

            order_id INTEGER PRIMARY KEY AUTOINCREMENT,

            user_id INTEGER NOT NULL,

            cylinder_type TEXT NOT NULL CHECK(cylinder_type IN ('Domestic','Commercial')),

            quantity INTEGER NOT NULL CHECK(quantity BETWEEN 1 AND 5),

            unit_price INTEGER NOT NULL,

            total_amount INTEGER NOT NULL,

            priority INTEGER NOT NULL DEFAULT 2 CHECK(priority BETWEEN 1 AND 3),

            status TEXT NOT NULL DEFAULT 'Pending'

              CHECK(status IN ('Pending','Scheduled','Out for Delivery','Delivered','Cancelled')),

            delivery_address TEXT NOT NULL,

            created_at TEXT NOT NULL,

            FOREIGN KEY(user_id) REFERENCES users(user_id)

        );

        CREATE TABLE IF NOT EXISTS complaints (

            complaint_id INTEGER PRIMARY KEY AUTOINCREMENT,

            user_id INTEGER NOT NULL,

            category TEXT NOT NULL CHECK(category IN ('Late Delivery','Damaged Cylinder','Payment Issue','Staff Behaviour','Other')),

            details TEXT NOT NULL,

            status TEXT NOT NULL DEFAULT 'Open' CHECK(status IN ('Open','In Progress','Resolved','Rejected')),

            created_at TEXT NOT NULL,

            FOREIGN KEY(user_id) REFERENCES users(user_id)

        );

        CREATE TABLE IF NOT EXISTS inventory (

            item_id INTEGER PRIMARY KEY AUTOINCREMENT,

            item_name TEXT NOT NULL UNIQUE,

            category TEXT NOT NULL CHECK(category IN ('Cylinder','Staff','Vehicle','Facility')),

            available INTEGER NOT NULL CHECK(available >= 0),

            unit TEXT NOT NULL

        );

        CREATE TABLE IF NOT EXISTS process_logs (

            log_id INTEGER PRIMARY KEY AUTOINCREMENT,

            order_id INTEGER NOT NULL,

            pid TEXT NOT NULL,

            algorithm TEXT NOT NULL,

            waiting_time INTEGER NOT NULL,

            turnaround_time INTEGER NOT NULL,

            created_at TEXT NOT NULL,

            FOREIGN KEY(order_id) REFERENCES orders(order_id)

        );

        CREATE TABLE IF NOT EXISTS password_resets (

            reset_id INTEGER PRIMARY KEY AUTOINCREMENT,

            user_id INTEGER NOT NULL,

            code_hash TEXT NOT NULL,

            expires_at REAL NOT NULL,

            used INTEGER NOT NULL DEFAULT 0,

            created_at TEXT NOT NULL,

            FOREIGN KEY(user_id) REFERENCES users(user_id)

        );

        """)

        # Migrate older databases without losing the ability to run the current app.

        user_columns = {r["name"] for r in conn.execute("PRAGMA table_info(users)")}

        if "account_status" not in user_columns:

            conn.execute("ALTER TABLE users ADD COLUMN account_status TEXT NOT NULL DEFAULT 'Active'")

        conn.execute("CREATE TABLE IF NOT EXISTS app_meta (meta_key TEXT PRIMARY KEY, meta_value TEXT NOT NULL)")

        # One-time demo reset: the earlier build seeded hundreds of rows. Reset only

        # when upgrading to this controlled 50-record dataset, not on every restart.

        seed_version = conn.execute("SELECT meta_value FROM app_meta WHERE meta_key='seed_version'").fetchone()

        if not seed_version or seed_version[0] != '50-records-v1':

            conn.execute("DELETE FROM process_logs")

            conn.execute("DELETE FROM complaints")

            conn.execute("DELETE FROM orders")

            conn.execute("DELETE FROM password_resets")

            conn.execute("DELETE FROM inventory")

            conn.execute("DELETE FROM users WHERE role='user'")

            conn.execute("DELETE FROM users WHERE role='admin' AND phone <> '9999999999'")

            conn.execute("INSERT OR REPLACE INTO app_meta(meta_key,meta_value) VALUES('seed_version','50-records-v1')")

        conn.execute("""INSERT OR IGNORE INTO users(name,phone,password,role,address,account_status,created_at)

                        VALUES(?,?,?,?,?,?,?)""",

                     ("GasGrid Admin","9999999999",generate_password_hash("Admin@123"),"admin","GasGrid Office, Dehradun","Active",now()))

        streets = ["Clement Town", "Rajpur Road", "Prem Nagar", "Patel Nagar", "Race Course",

                   "Dalanwala", "Vasant Vihar", "Sahastradhara Road", "Nehru Colony", "Indra Nagar"]

        names = ["Aarav Sharma", "Aditi Verma", "Vivaan Singh", "Ananya Gupta", "Aditya Joshi",

                 "Diya Rawat", "Arjun Kapoor", "Isha Mehta", "Reyansh Negi", "Saanvi Chauhan",

                 "Kabir Bhandari", "Myra Saini", "Atharv Rana", "Kiara Malik", "Ishaan Bisht",

                 "Pari Thakur", "Krishna Das", "Navya Jain", "Rudra Pant", "Sara Khan"]

        customers=[]

        # Exactly 50 customer records including the familiar demo customer login.

        conn.execute("""INSERT OR IGNORE INTO users(name,phone,password,role,address,account_status,created_at)

                        VALUES(?,?,?,?,?,?,?)""",

                     ("Demo Customer","9000000000",generate_password_hash("User@123"),"user","Clement Town, Dehradun","Active",now()))

        for i in range(1,50):

            phone=f"91{i:08d}"  # valid, unique 10-digit Indian mobile number

            name=names[(i-1)%len(names)] if i <= len(names) else f"Demo Customer {i+1:02d}"

            address=f"House {10+i}, {streets[(i-1)%len(streets)]}, Dehradun"

            status="Inactive" if i in (9,18,27,36,45) else ("Suspended" if i in (14,35) else "Active")

            conn.execute("""INSERT OR IGNORE INTO users(name,phone,password,role,address,account_status,created_at)

                            VALUES(?,?,?,?,?,?,?)""",

                         (name,phone,generate_password_hash("User@123"),"user",address,status,now()))

        customers=[dict(r) for r in conn.execute("SELECT user_id,name,phone,address,account_status FROM users WHERE role='user' ORDER BY user_id LIMIT 50")]

        # Exactly 50 focused inventory rows across the four operational categories.

        inventory_items=[]

        inventory_items += [(f"Domestic Cylinder Stock {i:02d}","Cylinder",80+i,"cylinders") for i in range(1,16)]

        inventory_items += [(f"Commercial Cylinder Stock {i:02d}","Cylinder",25+i,"cylinders") for i in range(1,11)]

        inventory_items += [(f"Delivery Staff Team {i:02d}","Staff",4+(i%5),"people") for i in range(1,11)]

        inventory_items += [(f"Delivery Vehicle Unit {i:02d}","Vehicle",1+(i%2),"vehicles") for i in range(1,8)]

        inventory_items += [(f"Dispatch Facility Slot {i:02d}","Facility",2+(i%4),"slots") for i in range(1,9)]

        for item in inventory_items:

            conn.execute("INSERT OR IGNORE INTO inventory(item_name,category,available,unit) VALUES(?,?,?,?)",item)

        # Exactly 50 default orders, distributed among the 50 customers.

        existing_orders=conn.execute("SELECT COUNT(*) FROM orders").fetchone()[0]

        if existing_orders == 0:

            customer_rows=conn.execute("SELECT user_id,address FROM users WHERE role='user' ORDER BY user_id LIMIT 50").fetchall()

            base_time=datetime.now()

            for i in range(50):

                cust=customer_rows[i]

                cylinder_type="Commercial" if i%7==0 else "Domestic"

                quantity=(i%3)+1

                unit_price=2200 if cylinder_type=="Commercial" else 950

                priority=(i%3)+1

                multiplier={1:1.15,2:1.0,3:0.95}[priority]

                total=int(unit_price*quantity*multiplier+0.5)

                status=["Pending","Scheduled","Out for Delivery","Delivered","Cancelled"][i%5]

                created=(base_time-timedelta(days=i*2,hours=i%12)).isoformat(timespec="seconds")

                conn.execute("""INSERT INTO orders(user_id,cylinder_type,quantity,unit_price,total_amount,priority,status,delivery_address,created_at)

                                VALUES(?,?,?,?,?,?,?,?,?)""",

                             (cust["user_id"],cylinder_type,quantity,unit_price,total,priority,status,cust["address"],created))

            # Keep enough pending work for a meaningful scheduling comparison.

            conn.execute("UPDATE orders SET status='Pending' WHERE order_id IN (SELECT order_id FROM orders ORDER BY order_id DESC LIMIT 15)")

        # Exactly 50 seed complaints on a clean seed. Never duplicate them on restart.

        if conn.execute("SELECT COUNT(*) FROM complaints").fetchone()[0] == 0:

            customer_rows=conn.execute("SELECT user_id FROM users WHERE role='user' ORDER BY user_id LIMIT 50").fetchall()

            categories=['Late Delivery','Damaged Cylinder','Payment Issue','Staff Behaviour','Other']

            statuses=['Open','In Progress','Resolved','Rejected']

            for i in range(50):

                conn.execute("INSERT INTO complaints(user_id,category,details,status,created_at) VALUES(?,?,?,?,?)",

                    (customer_rows[i]["user_id"],categories[i%len(categories)],

                     f"Demo support case {i+1:02d}: please review the refill service request.",

                     statuses[i%len(statuses)],(datetime.now()-timedelta(days=i)).isoformat(timespec="seconds")))

def login_required(role=None):

    def decorator(fn):

        @wraps(fn)

        def wrapper(*args, **kwargs):

            if not session.get("user_id"):

                return jsonify({"error":"Please log in first."}), 401

            if role and session.get("role") != role:

                return jsonify({"error":"You do not have permission for this action."}), 403

            return fn(*args, **kwargs)

        return wrapper

    return decorator

# Shared input validation prevents empty, oversized, or unexpected text values.

def clean_text(value, label, min_len=2, max_len=120):

    value = (value or "").strip()

    if len(value) < min_len or len(value) > max_len:

        raise ValueError(f"{label} must be between {min_len} and {max_len} characters.")

    if not re.fullmatch(r"[A-Za-z0-9 .,'/#()\\-]+", value):

        raise ValueError(f"{label} contains unsupported characters.")

    if not re.search(r"[A-Za-z]", value):

        raise ValueError(f"{label} must contain letters.")

    if re.search(r"(.)\1{5,}", value) or len(re.findall(r"[A-Za-z]", value)) < 2:

        raise ValueError(f"Please enter a valid {label.lower()}.")

    return value

def valid_phone(value):

    value = (value or "").strip()

    if not re.fullmatch(r"[6-9]\d{9}", value):

        raise ValueError("Enter a valid 10-digit Indian mobile number starting with 6, 7, 8 or 9.")

    return value

@app.get("/")

def home():

    if not session.get("user_id"):

        return redirect(url_for("login_page"))

    return render_template("index.html", role=session.get("role"), name=session.get("name"))

@app.get("/login")

def login_page():

    if session.get("user_id"):

        return redirect(url_for("home"))

    return render_template("login.html")

@app.post("/api/login")

def login():

    data = request.get_json(silent=True) or {}

    phone = (data.get("phone") or "").strip()

    password = data.get("password") or ""

    role = data.get("role")

    if role not in ("user","admin"):

        return jsonify({"error":"Choose User or Admin login."}), 400

    with connect_db() as conn:

        row = conn.execute("SELECT * FROM users WHERE phone=? AND role=?", (phone, role)).fetchone()

        # Support older demo databases: successful legacy plaintext logins are migrated to hashes.

        password_ok = False

        if row:

            stored = row["password"]

            try:

                password_ok = check_password_hash(stored, password)

            except (ValueError, TypeError):

                password_ok = False

            if not password_ok and stored == password:

                password_ok = True

                conn.execute("UPDATE users SET password=? WHERE user_id=?", (generate_password_hash(password), row["user_id"]))

    if not row or not password_ok:

        return jsonify({"error":"Incorrect phone, password, or account type."}), 401

    if row["role"] == "user" and row["account_status"] != "Active":

        return jsonify({"error":f"This customer account is {row['account_status'].lower()}. Please contact the administrator."}), 403

    session.clear()

    session.update(user_id=row["user_id"], role=row["role"], name=row["name"], phone=row["phone"])

    return jsonify({"message":"Login successful.","role":row["role"]})

@app.post("/api/register")

def register():

    data = request.get_json(silent=True) or {}

    try:

        name = clean_text(data.get("name"),"Name",2,60)

        phone = valid_phone(data.get("phone"))

        address = clean_text(data.get("address"),"Address",5,160)

        password = data.get("password") or ""

        if not re.fullmatch(r"(?=.*[A-Za-z])(?=.*\d)[A-Za-z\d!@#$%^&*_-]{8,40}", password):

            raise ValueError("Password must be 8-40 characters and include letters and numbers.")

    except ValueError as e:

        return jsonify({"error":str(e)}),400

    with connect_db() as conn:

        try:

            conn.execute("INSERT INTO users(name,phone,password,role,address,created_at) VALUES(?,?,?,?,?,?)",

                         (name,phone,generate_password_hash(password),"user",address,now()))

        except sqlite3.IntegrityError:

            return jsonify({"error":"This phone number already has an account. Please log in."}),409

    return jsonify({"message":"Account created. You can now log in."}),201

@app.post("/api/forgot-password")

def forgot_password():

    """Create a short-lived recovery code for the local academic demo.

    A real deployment must deliver this code through a verified SMS/email provider;

    this prototype returns it to the UI so the recovery flow can be tested offline.

    """

    data = request.get_json(silent=True) or {}

    phone = (data.get("phone") or "").strip()

    role = data.get("role")

    if role not in ("user", "admin") or not re.fullmatch(r"[6-9]\d{9}", phone):

        return jsonify({"error":"Enter a valid mobile number and account type."}), 400

    code = f"{secrets.randbelow(1_000_000):06d}"

    with connect_db() as conn:

        user = conn.execute("SELECT user_id FROM users WHERE phone=? AND role=?", (phone, role)).fetchone()

        if not user:

            return jsonify({"message":"If the account exists, a recovery code has been generated. Check the demo recovery panel."})

        conn.execute("UPDATE password_resets SET used=1 WHERE user_id=? AND used=0", (user["user_id"],))

        conn.execute("INSERT INTO password_resets(user_id,code_hash,expires_at,used,created_at) VALUES(?,?,?,?,?)",

                     (user["user_id"], generate_password_hash(code), (datetime.now()).timestamp() + 600, 0, now()))

    return jsonify({"message":"Recovery code generated. This local demo displays it here; production needs verified SMS/email delivery.", "demo_code":code})

@app.post("/api/reset-password")

def reset_password():

    """Validate a one-time recovery code and replace the password hash."""

    data = request.get_json(silent=True) or {}

    phone = (data.get("phone") or "").strip()

    role = data.get("role")

    code = (data.get("code") or "").strip()

    new_password = data.get("new_password") or ""

    if role not in ("user", "admin") or not re.fullmatch(r"[6-9]\d{9}", phone):

        return jsonify({"error":"Enter a valid mobile number and account type."}), 400

    if not re.fullmatch(r"(?=.*[A-Za-z])(?=.*\d)[A-Za-z\d!@#$%^&*_-]{8,40}", new_password):

        return jsonify({"error":"Password must be 8-40 characters and include letters and numbers."}), 400

    if not re.fullmatch(r"\d{6}", code):

        return jsonify({"error":"Enter the 6-digit recovery code."}), 400

    with connect_db() as conn:

        user = conn.execute("SELECT user_id FROM users WHERE phone=? AND role=?", (phone, role)).fetchone()

        if not user:

            return jsonify({"error":"Invalid or expired recovery code."}), 400

        reset = conn.execute("SELECT * FROM password_resets WHERE user_id=? AND used=0 ORDER BY reset_id DESC LIMIT 1",

                             (user["user_id"],)).fetchone()

        if not reset or reset["expires_at"] < datetime.now().timestamp():

            return jsonify({"error":"Invalid or expired recovery code. Request a new one."}), 400

        try:

            code_ok = check_password_hash(reset["code_hash"], code)

        except (ValueError, TypeError):

            code_ok = False

        if not code_ok:

            return jsonify({"error":"Invalid or expired recovery code."}), 400

        conn.execute("UPDATE users SET password=? WHERE user_id=?", (generate_password_hash(new_password), user["user_id"]))

        conn.execute("UPDATE password_resets SET used=1 WHERE reset_id=?", (reset["reset_id"],))

    return jsonify({"message":"Password reset successfully. You can now sign in."})

@app.post("/api/logout")

def logout():

    session.clear()

    return jsonify({"message":"Logged out."})

@app.get("/api/me")

@login_required()

def me():

    return jsonify({"user_id":session["user_id"],"role":session["role"],"name":session["name"],"phone":session["phone"]})

@app.get("/api/dashboard")

@login_required()

def dashboard():

    """Return role-scoped records; customers never receive other customers' data."""

    with connect_db() as conn:

        if session["role"] == "admin":

            stats = {

                "orders":conn.execute("SELECT COUNT(*) FROM orders").fetchone()[0],

                "pending":conn.execute("SELECT COUNT(*) FROM orders WHERE status IN ('Pending','Scheduled')").fetchone()[0],

                "delivered":conn.execute("SELECT COUNT(*) FROM orders WHERE status='Delivered'").fetchone()[0],

                "customers":conn.execute("SELECT COUNT(*) FROM users WHERE role='user'").fetchone()[0],

                "complaints":conn.execute("SELECT COUNT(*) FROM complaints WHERE status IN ('Open','In Progress')").fetchone()[0]

            }

            orders = [dict(r) for r in conn.execute("""SELECT o.*,u.name AS customer,u.phone

                FROM orders o JOIN users u ON u.user_id=o.user_id ORDER BY o.order_id DESC LIMIT 50""")]

            inventory = [dict(r) for r in conn.execute("SELECT * FROM inventory ORDER BY category,item_name")]

            complaints = [dict(r) for r in conn.execute("""SELECT c.*,u.name AS customer,u.phone FROM complaints c

                JOIN users u ON u.user_id=c.user_id ORDER BY c.complaint_id DESC""")]

            customers = [dict(r) for r in conn.execute("""SELECT user_id,name,phone,address,account_status,created_at FROM users

                WHERE role='user' ORDER BY user_id DESC""")]

        else:

            stats = {

                "orders":conn.execute("SELECT COUNT(*) FROM orders WHERE user_id=?",(session["user_id"],)).fetchone()[0],

                "pending":conn.execute("SELECT COUNT(*) FROM orders WHERE user_id=? AND status IN ('Pending','Scheduled','Out for Delivery')",(session["user_id"],)).fetchone()[0],

                "delivered":conn.execute("SELECT COUNT(*) FROM orders WHERE user_id=? AND status='Delivered'",(session["user_id"],)).fetchone()[0],

                "complaints":conn.execute("SELECT COUNT(*) FROM complaints WHERE user_id=?",(session["user_id"],)).fetchone()[0]

            }

            orders = [dict(r) for r in conn.execute("SELECT * FROM orders WHERE user_id=? ORDER BY order_id DESC",(session["user_id"],))]

            inventory, complaints, customers = [], [dict(r) for r in conn.execute("SELECT * FROM complaints WHERE user_id=? ORDER BY complaint_id DESC",(session["user_id"],))], []

        return jsonify({"stats":stats,"orders":orders,"inventory":inventory,"complaints":complaints,"customers":customers})

@app.post("/api/orders")

@login_required()

# Persist a single order with exactly one delivery_address and reserve stock atomically.

def create_order():

    data = request.get_json(silent=True) or {}

    try:

        cylinder_type = data.get("cylinder_type")

        if cylinder_type not in ("Domestic","Commercial"):

            raise ValueError("Select Domestic or Commercial cylinder.")

        quantity = int(data.get("quantity",0))

        priority = int(data.get("priority",2))

        if not 1 <= quantity <= 5:

            raise ValueError("Quantity must be between 1 and 5 cylinders per order.")

        if priority not in (1,2,3):

            raise ValueError("Choose a valid delivery priority.")

        address = clean_text(data.get("address"),"Delivery address",5,160)

    except (ValueError,TypeError) as e:

        return jsonify({"error":str(e) or "Please check the refill details."}),400

    # Priority multipliers discourage unnecessary high-priority requests while rewarding low priority.

    unit_price = 950 if cylinder_type == "Domestic" else 2200

    priority_multiplier = {1: 1.15, 2: 1.00, 3: 0.95}[priority]

    base_total = unit_price * quantity

    total = int(base_total * priority_multiplier + 0.5)

    with connect_db() as conn:

        stock_name = cylinder_type + " LPG Cylinders"

        stock = conn.execute("SELECT available FROM inventory WHERE item_name=?",(stock_name,)).fetchone()

        if not stock or stock["available"] < quantity:

            return jsonify({"error":f"Insufficient {cylinder_type.lower()} cylinder stock. Available: {stock['available'] if stock else 0}."}),409

        # Save the current delivery address to the customer profile and create the order.

        conn.execute("UPDATE users SET address=? WHERE user_id=?",(address,session["user_id"]))

        cur = conn.execute("""INSERT INTO orders(user_id,cylinder_type,quantity,unit_price,total_amount,priority,status,delivery_address,created_at)

            VALUES(?,?,?,?,?,?,?,?,?)""",

            (session["user_id"],cylinder_type,quantity,unit_price,total,priority,"Pending",address,now()))

        order_id = cur.lastrowid

        conn.execute("UPDATE inventory SET available=available-? WHERE item_name=?",

                     (quantity, stock_name))

    return jsonify({"message":"Refill order saved successfully.","order_id":order_id,"pid":f"P{order_id}","unit_price":unit_price,"base_total":base_total,"priority_multiplier":priority_multiplier,"total_amount":total}),201

@app.post("/api/complaints")

@login_required()

def create_complaint():

    data = request.get_json(silent=True) or {}

    category = data.get("category")

    try:

        details = clean_text(data.get("details"),"Complaint details",10,500)

        if category not in ("Late Delivery","Damaged Cylinder","Payment Issue","Staff Behaviour","Other"):

            raise ValueError("Select a valid complaint category.")

    except ValueError as e:

        return jsonify({"error":str(e)}),400

    with connect_db() as conn:

        cur = conn.execute("INSERT INTO complaints(user_id,category,details,status,created_at) VALUES(?,?,?,?,?)",

                           (session["user_id"],category,details,"Open",now()))

        cid = cur.lastrowid

    return jsonify({"message":"Complaint submitted.","complaint_id":cid}),201

@app.post("/api/admin/customer-status")

@login_required("admin")

def update_customer_status():

    data=request.get_json(silent=True) or {}

    try:

        user_id=int(data.get("user_id"))

        status=data.get("status")

        if status not in ("Active","Inactive","Suspended"):

            raise ValueError("Choose Active, Inactive, or Suspended.")

    except (TypeError,ValueError) as exc:

        return jsonify({"error":str(exc) or "Invalid customer status."}),400

    with connect_db() as conn:

        cur=conn.execute("UPDATE users SET account_status=? WHERE user_id=? AND role='user'",(status,user_id))

        if cur.rowcount == 0:

            return jsonify({"error":"Customer not found."}),404

    return jsonify({"message":f"Customer account marked {status.lower()}."})

@app.post("/api/admin/order-status")

@login_required("admin")

def order_status():

    data = request.get_json(silent=True) or {}

    try:

        order_id = int(data.get("order_id"))

    except (TypeError,ValueError):

        return jsonify({"error":"Enter a valid order ID."}),400

    status = data.get("status")

    if status not in ("Pending","Scheduled","Out for Delivery","Delivered","Cancelled"):

        return jsonify({"error":"Invalid order status."}),400

    with connect_db() as conn:

        row = conn.execute("SELECT * FROM orders WHERE order_id=?",(order_id,)).fetchone()

        if not row:

            return jsonify({"error":"Order not found."}),404

        if row["status"] == "Cancelled" and status != "Cancelled":

            return jsonify({"error":"Cancelled orders cannot be reopened in this demo."}),400

        if status == "Cancelled" and row["status"] != "Cancelled":

            conn.execute("UPDATE inventory SET available=available+? WHERE item_name=?",

                         (row["quantity"],row["cylinder_type"]+" LPG Cylinders"))

        elif row["status"] == "Cancelled" and status != "Cancelled":

            pass

        conn.execute("UPDATE orders SET status=? WHERE order_id=?",(status,order_id))

    return jsonify({"message":f"Order #{order_id} updated to {status}."})

@app.post("/api/admin/inventory")

@login_required("admin")

def update_inventory():

    """Apply a bounded inventory adjustment and prevent negative stock."""

    data = request.get_json(silent=True) or {}

    try:

        item_id = int(data.get("item_id"))

        change = int(data.get("change"))

        if change == 0 or abs(change) > 500:

            raise ValueError("Stock change must be between -500 and 500, excluding zero.")

    except (TypeError,ValueError) as e:

        return jsonify({"error":str(e) or "Enter a valid stock adjustment."}),400

    with connect_db() as conn:

        row = conn.execute("SELECT * FROM inventory WHERE item_id=?",(item_id,)).fetchone()

        if not row:

            return jsonify({"error":"Inventory item not found."}),404

        if row["available"] + change < 0:

            return jsonify({"error":"Stock cannot become negative."}),400

        conn.execute("UPDATE inventory SET available=available+? WHERE item_id=?",(change,item_id))

    return jsonify({"message":f"{row['item_name']} stock updated."})

@app.post("/api/admin/complaint-status")

@login_required("admin")

def complaint_status():

    data = request.get_json(silent=True) or {}

    try:

        complaint_id = int(data.get("complaint_id"))

    except (TypeError,ValueError):

        return jsonify({"error":"Enter a valid complaint ID."}),400

    status = data.get("status")

    if status not in ("Open","In Progress","Resolved","Rejected"):

        return jsonify({"error":"Invalid complaint status."}),400

    with connect_db() as conn:

        cur = conn.execute("UPDATE complaints SET status=? WHERE complaint_id=?",(status,complaint_id))

        if cur.rowcount == 0:

            return jsonify({"error":"Complaint not found."}),404

    return jsonify({"message":f"Complaint #{complaint_id} updated."})

def _schedule_jobs(jobs, algorithm, quantum=2):

    """Run one of the five scheduling algorithms and return a Gantt trace + metrics.

    Each Gantt entry represents a continuous execution slice. Preemptive algorithms

    may therefore return several slices for the same process.

    """

    jobs = [dict(j, remaining=j["burst"]) for j in jobs]

    gantt = []

    completion = {}

    def add_slice(job, start, end, merge=True):

        if end <= start:

            return

        # Preserve each Round Robin quantum boundary, even when the same process runs twice.

        if merge and gantt and gantt[-1]["pid"] == job["pid"] and gantt[-1]["end"] == start:

            gantt[-1]["end"] = end

        else:

            gantt.append({"pid": job["pid"], "start": start, "end": end})

    if algorithm in ("FCFS", "SJF", "PRIORITY"):

        time = 0

        pending = jobs[:]

        while pending:

            ready = [j for j in pending if j["arrival"] <= time]

            if not ready:

                time = min(j["arrival"] for j in pending)

                ready = [j for j in pending if j["arrival"] <= time]

            if algorithm == "FCFS":

                job = min(ready, key=lambda j: (j["arrival"], j["order_id"]))

            elif algorithm == "SJF":

                job = min(ready, key=lambda j: (j["burst"], j["arrival"], j["order_id"]))

            else:  # Priority 1 is High, 2 Normal, 3 Low.

                job = min(ready, key=lambda j: (j["priority"], j["arrival"], j["order_id"]))

            start = time

            time += job["burst"]

            add_slice(job, start, time)

            completion[job["pid"]] = time

            pending.remove(job)

    elif algorithm == "SRTF":

        time = min(j["arrival"] for j in jobs)

        remaining = {j["pid"]: j["burst"] for j in jobs}

        while len(completion) < len(jobs):

            ready = [j for j in jobs if j["arrival"] <= time and remaining[j["pid"]] > 0]

            if not ready:

                time = min(j["arrival"] for j in jobs if remaining[j["pid"]] > 0)

                continue

            job = min(ready, key=lambda j: (remaining[j["pid"]], j["arrival"], j["order_id"]))

            add_slice(job, time, time + 1)

            remaining[job["pid"]] -= 1

            time += 1

            if remaining[job["pid"]] == 0:

                completion[job["pid"]] = time

    elif algorithm == "RR":

        time = min(j["arrival"] for j in jobs)

        remaining = {j["pid"]: j["burst"] for j in jobs}

        ready_queue = []

        arrived = set()

        while len(completion) < len(jobs):

            for j in sorted(jobs, key=lambda x: (x["arrival"], x["order_id"])):

                if j["arrival"] <= time and j["pid"] not in arrived and remaining[j["pid"]] > 0:

                    ready_queue.append(j)

                    arrived.add(j["pid"])

            if not ready_queue:

                future = [j["arrival"] for j in jobs if remaining[j["pid"]] > 0 and j["pid"] not in arrived]

                if not future:

                    break

                time = min(future)

                continue

            job = ready_queue.pop(0)

            run_for = min(quantum, remaining[job["pid"]])

            start = time

            end = time + run_for

            add_slice(job, start, end, merge=False)

            time = end

            remaining[job["pid"]] -= run_for

            # Add any jobs that arrived during this time slice before re-queueing the current process.

            for j in sorted(jobs, key=lambda x: (x["arrival"], x["order_id"])):

                if j["arrival"] <= time and j["pid"] not in arrived and remaining[j["pid"]] > 0:

                    ready_queue.append(j)

                    arrived.add(j["pid"])

            if remaining[job["pid"]] == 0:

                completion[job["pid"]] = time

            else:

                ready_queue.append(job)

    results = []

    for job in jobs:

        finish = completion[job["pid"]]

        turnaround = finish - job["arrival"]

        results.append({**{k:v for k,v in job.items() if k != "remaining"},

                        "start": min(g["start"] for g in gantt if g["pid"] == job["pid"]),

                        "completion": finish,

                        "turnaround": turnaround,

                        "waiting": turnaround - job["burst"]})

    results.sort(key=lambda x: x["order_id"])

    return results, gantt

@app.post("/api/schedule")

@login_required("admin")

def schedule():

    data = request.get_json(silent=True) or {}

    algorithm = data.get("algorithm", "FCFS")

    if algorithm not in ("FCFS", "SJF", "SRTF", "RR", "PRIORITY"):

        return jsonify({"error":"Unsupported scheduling algorithm."}), 400

    try:

        quantum = int(data.get("quantum", 2)) if algorithm == "RR" else 2

    except (TypeError, ValueError):

        return jsonify({"error":"Round Robin time quantum must be a whole number from 1 to 10."}), 400

    if algorithm == "RR" and not 1 <= quantum <= 10:

        return jsonify({"error":"Round Robin time quantum must be a whole number from 1 to 10."}), 400

    with connect_db() as conn:

        rows = conn.execute("""SELECT order_id,priority,created_at,quantity,cylinder_type FROM orders

            WHERE status IN ('Pending','Scheduled') ORDER BY created_at,order_id""").fetchall()

    if not rows:

        return jsonify({"algorithm":algorithm,"message":"No pending orders to schedule.","results":[],"gantt":[]})

    # Simulation arrivals are batched in groups of three so several orders can

    # compete for the CPU at once. Burst is an explicit teaching estimate based

    # on order size/type, not elapsed real-world delivery time.

    jobs = [{"pid":f"P{r['order_id']}", "order_id":r["order_id"],

             "arrival":i // 3,

             "burst":int(r["quantity"]) + (2 if r["cylinder_type"] == "Commercial" else 1),

             "priority":r["priority"]} for i,r in enumerate(rows)]

    results, gantt = _schedule_jobs(jobs, algorithm, quantum)

    with connect_db() as conn:

        for r in results:

            conn.execute("INSERT INTO process_logs(order_id,pid,algorithm,waiting_time,turnaround_time,created_at) VALUES(?,?,?,?,?,?)",

                         (r["order_id"],r["pid"],algorithm,r["waiting"],r["turnaround"],now()))

            conn.execute("UPDATE orders SET status='Scheduled' WHERE order_id=? AND status='Pending'", (r["order_id"],))

    return jsonify({"algorithm":algorithm,"quantum":quantum if algorithm == "RR" else None,

                    "results":results,"gantt":gantt,

                    "average_waiting":round(sum(r["waiting"] for r in results)/len(results),2),

                    "average_turnaround":round(sum(r["turnaround"] for r in results)/len(results),2)})

@app.get("/api/schedule/history")

@login_required()

def schedule_history():

    with connect_db() as conn:

        if session["role"] == "admin":

            rows = conn.execute("""SELECT p.*,o.delivery_address FROM process_logs p JOIN orders o ON o.order_id=p.order_id

                                   ORDER BY p.log_id DESC LIMIT 50""").fetchall()

        else:

            rows = conn.execute("""SELECT p.*,o.delivery_address FROM process_logs p JOIN orders o ON o.order_id=p.order_id

                                   WHERE o.user_id=? ORDER BY p.log_id DESC LIMIT 50""",(session["user_id"],)).fetchall()

    return jsonify([dict(r) for r in rows])

if __name__ == "__main__":

    init_db()

    app.run(host="127.0.0.1", port=5000, debug=False)
