"""Configuration for the GasGrid Flask application.

The supplied implementation currently uses SQLite to preserve its original
runtime behavior. Oracle educational scripts are kept under database/dbms_concepts.
"""
import os
from pathlib import Path

BASE_DIR = Path(__file__).resolve().parent
SECRET_KEY = os.environ.get("GASGRID_SECRET", "change-this-secret-for-real-deployment")
DATABASE_PATH = BASE_DIR / "gasgrid.db"
HOST = os.environ.get("GASGRID_HOST", "127.0.0.1")
PORT = int(os.environ.get("GASGRID_PORT", "5000"))
