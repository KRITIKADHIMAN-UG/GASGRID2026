"""Business service entry points. Existing routes and business logic remain in app.py.
Move individual operations here only when intentionally refactoring in a later version.
"""

def project_modules():
    return ["authentication", "orders", "complaints", "inventory", "CPU scheduling"]
