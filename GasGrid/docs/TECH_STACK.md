# GasGrid Technology Stack

This document describes the stack used by the **working application in this repository**.

| Layer | Technology | Purpose |
|---|---|---|
| Frontend | HTML5, CSS3, JavaScript | Forms, dashboards, filters, timeline/Gantt display |
| Backend | Python, Flask | Routes, sessions, validation, booking and admin logic |
| Database | SQLite | Persistent local data, constraints, queries, and transactions |
| Password security | Werkzeug password hashing | Store password hashes rather than plain-text passwords |
| OS scheduling module | Python scheduling simulation | FCFS, SJF, SRTF, Round Robin, Priority and Gantt timeline |
| Test suite | Python `unittest` | Scheduling logic checks |
| Version control | Git and GitHub | Source control and collaboration |
| Local launcher | Windows batch (`.bat`) | Create virtual environment, install requirements, start Flask |

## Important stack clarification

The screenshot supplied with the request lists **Oracle SQL/PL/SQL, `python-oracledb`, and C/GCC**. The current working website, however, uses SQLite and implements its scheduling simulation in Python. This repository documents the actual stack rather than claiming Oracle or C is integrated when it is not.

Switching the existing live application from SQLite to Oracle and replacing/adding the OS simulator in C would be a substantive backend change, not just a folder-layout or documentation change. It has deliberately not been done here so the current website behaviour remains unchanged.
