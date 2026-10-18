"""Reusable validation helpers for the GasGrid prototype."""
import re

def valid_phone(value):
    value = (value or "").strip()
    if not re.fullmatch(r"[6-9]\d{9}", value):
        raise ValueError("Enter a valid 10-digit Indian mobile number.")
    return value

def valid_password(value):
    if not re.fullmatch(r"(?=.*[A-Za-z])(?=.*\d)[A-Za-z\d!@#$%^&*_-]{8,40}", value or ""):
        raise ValueError("Password must be 8-40 characters and include letters and numbers.")
    return value
