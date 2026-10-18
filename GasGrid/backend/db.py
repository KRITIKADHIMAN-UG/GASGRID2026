"""Database compatibility helpers. The original app uses SQLite.
This module is provided as a clean place for database helpers in the requested structure.
"""
import sqlite3
from pathlib import Path

DB_PATH = Path(__file__).resolve().parent / "gasgrid.db"

def connect_db():
    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row
    conn.execute("PRAGMA foreign_keys = ON")
    return conn
