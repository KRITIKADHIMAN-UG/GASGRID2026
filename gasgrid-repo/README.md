# GasGrid: Smart LPG Management System

LPG booking and distribution platform for two cylinder types (14.2 kg domestic and 19 kg commercial, each with its own price, stock and bookings) with an **Oracle SQL** database, a **Java** REST backend and a **C** operating-system simulation engine.
All data shipped with the project is **sample data**.

```
 Browser (frontend/)  ──HTTP/JSON──►  Java backend (backend/)  ──JDBC──►  Oracle (database/)
                                            │
                                            └──stdin/stdout JSON──►  C OS engine (os/bin/*)
```

## Repository layout
```
gasgrid/
├── backend/                 Java 8+ / Maven. com.sun.net.httpserver, JDBC, no frameworks
│   └── src/main/java/gasgrid/
│       ├── Main.java            server, routing, static files
│       ├── controller/          Auth, Booking, Service, Account, Admin, Os
│       ├── service/             AuthService, BookingService, OsService (validation + engine input)
│       ├── dao/                 User, Booking, Service, Account, Report (JDBC + stored procedures)
│       ├── model/               User, Session
│       └── util/                Db, Json, Http, Security, OsBridge, SqlRunner, DbInit, Paging, Config
├── database/                Oracle SQL scripts (00 user, 01 tables, 02 indexes/views, 03 procedures,
│                            04 triggers, 05 seed data, 06 reports, 07 transaction demo, run_all.sql)
├── os/                      C OS engine: src/*.c, Makefile, build.bat, tests/
├── frontend/index.html      browser UI (standalone demo, see "Frontend status")
├── scripts/                 start.bat, stop.bat, start.sh
├── docs/                    API.md, DATABASE.md, OS_ENGINE.md
└── .github/workflows/ci.yml builds and tests on every push
```

## Prerequisites
* JDK 8 or newer and Maven 3.6+
* GCC (on Windows: MinGW-w64, `gcc` on PATH)
* Oracle Database 11g XE or newer, listener on `localhost:1521`, service `XE`

## Setup
1. **Create the DB user** (once, as SYSTEM): `sqlplus system@XE @database/00_create_user.sql`
2. `copy .env.example .env` and adjust DB URL/user/password if needed.
3. **Run:** `scripts\start.bat` (Linux/macOS: `scripts/start.sh`). It builds the C engine and the backend, then starts the server on <http://localhost:8080>.
   On first start the backend runs `database/01..05`, creates the schema, loads the sample data and sets the demo passwords. Later starts skip existing objects and never duplicate data.
4. Stop with `scripts\stop.bat`.

Manual alternative: `make -C os`, `cd backend && mvn package`, `java -jar target/gasgrid.jar`. To load the database by hand: `sqlplus gasgrid/gasgrid@XE @database/run_all.sql` (then start the backend once to set demo passwords).

## Demo accounts (sample data)
| Role | Email | Password |
|---|---|---|
| Admin | admin@gasgrid.in | admin123 |
| Distributor | distributor@gasgrid.in | distributor123 |
| Technician | technician@gasgrid.in | technician123 |
| Customer | customer@gasgrid.in | customer123 |

The 100 other sample customers are `cust1@sample.in` ... `cust100@sample.in` with password `sample123`.

## Sample data volumes
2 cylinder types, 101 customers + 3 staff users, 150 connections, 200 bookings, 100 deliveries, 15 warehouses, 300 inventory records, 30 workers, 15 technicians, 25 vehicles, 50 service requests, 100 reward transactions, 50 OS simulation records, 100 simulated processes, across Dehradun, Roorkee, Delhi, Jaipur, Chandigarh, Lucknow, Mumbai, Pune, Bengaluru and Hyderabad.

## Testing
* OS engine: `make -C os test` (15 checks against textbook answers).
* Backend: `mvn package` compiles it. See docs/API.md for `curl` examples to exercise the endpoints.

## Security notes
PBKDF2-HMAC-SHA256 password hashes (random salt), HttpOnly + SameSite=Strict session cookie, role checks on every route (server side), `Content-Type: application/json` required on writes, parameterised SQL everywhere, request size limit, login lock-out after 5 failures per 15 minutes, server bound to localhost, OS engine launched only from a fixed whitelist with no shell and all input validated and range-checked. Demo-only shortcuts: the password-reset token is returned in the response instead of being e-mailed, and sessions are held in memory.

## Frontend
`frontend/index.html` is the standalone browser app (data in localStorage, OS simulations in JavaScript). It has the two-cylinder-type booking flow, sign in / register / **Forgot Password** (reset code, shown in a simulated inbox because no email server exists), role-based Information Centre and the redesigned OS Control Centre (cards with ⓘ explanations). It is served by the backend but does **not** call the API yet; wiring its screens to `/api/*` is the next step. The backend (including `/api/auth/forgot` and `/api/auth/reset`) can be exercised now with `curl`.

## Known limitations
* The SQL, the PL/SQL and the JDBC layer were written for Oracle 11g XE but **have not been run against a live Oracle database** by the author of this scaffold; expect to fix small issues on first run (the backend prints PL/SQL compile errors at startup).
* No connection pool, no HTTPS, no e-mail sending, no automated Java unit tests.
