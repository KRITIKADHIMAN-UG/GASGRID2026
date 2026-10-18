# Repository Guide

```text
GasGrid/
├── app.py                    # Flask routes, database setup, validation and business logic
├── requirements.txt          # Python dependencies
├── run_gasgrid.bat           # Windows setup and launch script
├── .env.example              # Example configuration; no real secrets
├── .gitignore                # Excludes local database and generated files
├── LICENSE
├── README.md                 # Setup, demo credentials, features and tests
├── CONTRIBUTING.md
├── docs/
│   ├── TECH_STACK.md         # Actual technology stack and clarification
│   └── REPOSITORY_GUIDE.md
├── templates/
│   ├── login.html            # Login and password-recovery UI
│   └── index.html            # Main dashboard
├── static/
│   ├── app.js                # Dashboard interactions and scheduling chart
│   ├── login.js              # Login and password-recovery interactions
│   └── style.css             # Shared styling
└── tests/
    └── test_scheduling.py    # Scheduling algorithm tests
```

## Why `app.py` stays at the root

Flask expects `templates/` and `static/` alongside the application by default. Keeping this simple conventional layout avoids path changes and helps preserve the current working website.
