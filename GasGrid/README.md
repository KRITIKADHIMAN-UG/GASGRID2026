# GasGrid: Smart LPG Booking & Distribution Management System

Academic prototype for LPG booking and distribution management, including customer/admin login, refill orders, complaints, inventory management, and an OS CPU scheduling simulation.

## Stack represented in this package
- Frontend: HTML5, CSS3, JavaScript
- Backend: Python and Flask
- Runtime database in the supplied backend: SQLite (preserved from the uploaded source)
- DBMS learning scripts: Oracle SQL and PL/SQL under `database/dbms_concepts/`
- OS concepts: standalone C programs under `os_concepts/`
- Version control: Git and GitHub

> Important: the source file provided for this packaging task explicitly uses SQLite. Its database logic has been preserved rather than silently converted to Oracle. The Oracle SQL scripts are educational scripts and are not wired to the current Flask runtime. To make Oracle the live application database, the backend database layer would need a separate conversion.

## Run on Windows
1. Install Python 3.10+ and GCC/MinGW if you want to run the C programs.
2. Open PowerShell in the `GasGrid` folder.
3. Create a virtual environment: `py -m venv .venv`
4. Activate it: `.venv\Scripts\Activate.ps1`
5. Install dependencies: `python -m pip install -r requirements.txt`
6. Start the app: `python backend\app.py`
7. Open `http://127.0.0.1:5000`

Demo credentials from the supplied backend:
- Admin phone: `9999999999`, password: `Admin@123`
- Customer phone: `9000000000`, password: `User@123`

The app initializes its local database when started directly. Do not use demo credentials or the development secret in production.
