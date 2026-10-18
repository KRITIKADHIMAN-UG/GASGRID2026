# Project Architecture

- `frontend/`: HTML pages and static CSS/JavaScript assets.
- `backend/app.py`: existing Flask routes, authentication/session handling, SQLite persistence, order/complaint/inventory actions, and the CPU-scheduling API.
- `backend/config.py`, `db.py`, `services.py`, `validators.py`: supporting modules in the requested structure. The original business logic is retained in `app.py` to avoid a risky rewrite.
- `database/dbms_concepts/`: Oracle SQL/PLSQL practice scripts.
- `os_concepts/`: standalone C programs illustrating OS concepts.

The original source provided for packaging states that SQLite is intentional for the local prototype. Oracle SQL scripts are included for DBMS coursework but are not yet wired into the Flask backend.
