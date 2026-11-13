# GasGrid REST API

Base URL `http://localhost:8080/api`. JSON in/out. Writes need header `Content-Type: application/json`. Session via cookie `GG_SESSION` (set by login). Errors: `{"error":"message"}`.

```
curl -c jar -H "Content-Type: application/json" -d '{"email":"customer@gasgrid.in","password":"customer123"}' localhost:8080/api/auth/login
curl -b jar localhost:8080/api/bookings
curl -b jar -H "Content-Type: application/json" -d '{"connection_id":1,"cyl_type":"14.2KG"}' localhost:8080/api/bookings
```

| Method & path | Roles | Body / query | Notes |
|---|---|---|---|
| POST /auth/login | any | email, password | 401 wrong creds, 429 locked out |
| POST /auth/register | any | name, email, phone?, city?, password | creates a CUSTOMER |
| POST /auth/logout | logged in | | |
| POST /auth/forgot | any | email | demo returns `demo_reset_token` |
| POST /auth/reset | any | token, password | |
| GET /me | logged in | | |
| GET /bookings | all (customers see own) | status, q, page, size | paginated |
| POST /bookings | CUSTOMER | connection_id, cyl_type (14.2KG or 19KG) | 201, 409 DUPLICATE_BOOKING / OUT_OF_STOCK |
| POST /bookings/{id}/cancel | CUSTOMER (own), ADMIN | | only while BOOKED |
| POST /bookings/{id}/status | DISTRIBUTOR, ADMIN | status | SCHEDULED, OUT_FOR_DELIVERY, DELIVERED in order |
| GET /bookings/{id}/track | owner, staff | | delivery status, worker, vehicle |
| GET /bookings/types | logged in | | the two cylinder types (14.2KG domestic, 19KG commercial) with price and availability |
| GET, POST /bookings/connections | CUSTOMER | address, city, pin | |
| GET, POST /service | CUSTOMER (own) / TECHNICIAN, ADMIN (queue) | issue, priority 1-3 | queue ordered by priority |
| POST /service/{id}/assign | ADMIN | | picks least-loaded technician |
| POST /service/{id}/complete | TECHNICIAN, ADMIN | | |
| GET /account/rewards | CUSTOMER | | balance + history |
| POST /account/redeem | CUSTOMER | points | |
| GET, POST /account/notifications | logged in | | POST marks all read |
| GET, POST /account/family | CUSTOMER | connection_id, name, relation, can_book | |
| POST /account/family-permission | CUSTOMER | family_id, can_book | |
| GET /admin/stats, /users, /audit, /reports/{city,status,top,monthly} | ADMIN | page, size, q | |
| GET /admin/inventory, /inventory/summary, /vehicles, /workers | ADMIN, DISTRIBUTOR | warehouse, page | |
| POST /admin/price | ADMIN | cyl_type, price | separate price per cylinder type (100-10000) |
| GET /admin/inventory/types, /admin/reports/types | ADMIN (DISTRIBUTOR for inventory) | | stock, price, bookings and revenue per type |
| POST /admin/restock | ADMIN, DISTRIBUTOR | inv_id, qty | |
| POST /os/{cpu,process,sync,deadlock,memory,disk} | logged in | see OS_ENGINE.md | runs the C engine, logs to OS_SIMULATIONS |
| GET /os/history | logged in | page, size | admins see all |
