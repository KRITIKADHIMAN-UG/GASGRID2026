# 🔥 GasGrid – Smart LPG Management & Distribution System

**An Integrated LPG Booking, Inventory Management, and Operating System Simulation Platform**

GasGrid is a web-based application designed to simplify LPG booking and distribution while demonstrating the practical implementation of Database Management Systems (DBMS) and Operating System (OS) concepts.

The platform connects customers, distributors, technicians, and administrators through a centralized system that manages cylinder bookings, inventory, service requests, and resource allocation.

It also features an interactive OS Simulation Centre where users can explore and execute various operating system algorithms through real-world LPG distribution scenarios.

---

## 🚀 Key Features

### 1. LPG Management System
- User registration and secure login.
- Role-based dashboards for Customers, Distributors, Technicians, and Administrators.
- Support for **14.2 kg Domestic** and **19 kg Commercial** LPG cylinders.
- Online cylinder booking and cancellation.
- Booking history and real-time status tracking.
- Distributor-wise inventory management.
- Separate stock, pricing, and availability management.
- Mechanic and service request management.
- Rewards and redemption system.

### 2. Database Management System (DBMS)
- Centralized relational database.
- Structured tables with Primary Keys and Foreign Keys.
- Data integrity through constraints.
- SQL queries and PL/SQL operations.
- Transaction management.
- Views and indexes for efficient data retrieval.
- Inventory consistency and prevention of duplicate or invalid bookings.
- Proper handling of reserved and available stock.

### 3. Operating System Simulation Centre

GasGrid includes an interactive OS Control Centre that demonstrates fundamental OS concepts using simulated LPG distribution tasks and resource-allocation scenarios.

**CPU Scheduling Algorithms**
- First Come First Serve (FCFS)
- Shortest Job First (SJF)
- Shortest Remaining Time First (SRTF)
- Priority Scheduling
- Round Robin (RR)
- Multilevel Queue (MLQ)
- Multilevel Feedback Queue (MLFQ)

**Process Management**
- Process creation and termination.
- Process Control Block (PCB).
- Process states and transitions.
- Process suspension and resumption.
- Threads and scheduling.

**Process Synchronization**
- Critical section problem.
- Mutex locks and semaphores.
- Shared resource management.

**Deadlock Management**
- Deadlock detection.
- Deadlock prevention concepts.
- Resource Allocation Graph (RAG).
- Banker's Algorithm for deadlock avoidance.

**Memory Management**
- Memory allocation and deallocation.
- Memory partitioning.
- Paging and page replacement.

**Page Replacement Algorithms**
- FIFO
- LRU
- Optimal

**Disk Scheduling Algorithms**
- FCFS
- SSTF
- SCAN
- C-SCAN
- LOOK
- C-LOOK

**File Management**
- Sequential allocation.
- Linked allocation.
- Indexed allocation.

The simulator provides algorithm execution, process information, Gantt charts, performance metrics, and resource-allocation visualizations wherever applicable.

### 4. Information Centre
- Role-specific activity summaries.
- Booking and inventory statistics.
- Task and service history.
- System activity overview.
- OS simulation results and resource utilization.

---

## 🛠️ Technology Stack

| Component | Technologies |
|---|---|
| Frontend | HTML5, CSS3, JavaScript |
| Backend | Python, Flask |
| Database | MySQL |
| OS Simulation Engine | C |
| API Communication | REST APIs, JSON |
| Development Tools | VS Code, Git, GitHub |

---

## 🏗️ System Architecture

GasGrid follows a layered architecture connecting the frontend, backend, database, and OS simulation engine.

```text
             USER
               |
               v
       FRONTEND INTERFACE
         HTML / CSS / JS
               |
               v
        PYTHON FLASK API
          /          \
         /            \
        v              v
   MYSQL DATABASE    C OS ENGINE
   DBMS OPERATIONS   OS SIMULATIONS
         |              |
         v              v
   LPG MANAGEMENT   ALGORITHM RESULTS
         \              /
          \            /
           v          v
          USER INTERFACE
```

---

## 📂 Project Modules

| Module | Description |
|---|---|
| Authentication | Registration, login and password recovery |
| Customer | Cylinder booking, history and rewards |
| Distributor | Inventory and cylinder allocation |
| Technician | Service and maintenance requests |
| Admin | User and system management |
| DBMS | Data storage, queries and transactions |
| OS Simulator | Scheduling, memory, disk and process management |
| Information Centre | User-specific activities and statistics |

---

## ⚙️ Installation and Setup

### Prerequisites
- Python 3.x
- MySQL Server
- C Compiler (GCC recommended)
- Git
- Modern Web Browser

### 1. Clone the Repository

```bash
git clone <repository-url>
cd GasGrid
```

### 2. Create a Virtual Environment

```bash
python -m venv venv
```

Activate it:

**Windows**
```bash
venv\Scripts\activate
```

### 3. Install Dependencies

```bash
pip install -r requirements.txt
```

### 4. Configure the Database

- Start your MySQL server.
- Create the GasGrid database.
- Configure database credentials in the project's environment configuration.
- Execute the provided SQL schema and initialization scripts.

### 5. Compile the OS Simulation Engine

Compile the C programs using GCC as required by the project configuration.

```bash
gcc -o os_engine os_engine.c
```

### 6. Run the Application

```bash
python app.py
```

Open your browser and navigate to:

```text
http://127.0.0.1:5000
```

*Note: Adjust file names and database configuration according to the actual project structure.*

---

## 🎯 Project Objectives

- Digitize and simplify LPG booking and distribution.
- Improve inventory accuracy and resource utilization.
- Demonstrate practical DBMS concepts through a real-world application.
- Implement and visualize Operating System algorithms.
- Understand process scheduling, synchronization, memory management, and deadlock handling.
- Integrate C-based OS simulations with a Python Flask application.

---

## 🔮 Future Scope

- Online payment gateway integration.
- SMS and email notifications.
- Advanced distribution and route optimization.
- Predictive inventory management.
- Enhanced analytics and reporting.
- Cloud deployment and scalability.

---

## 👩‍💻 Project Information

**Project Name:** GasGrid – Smart LPG Management System

**Domain:** Full-Stack Development | Database Management Systems | Operating Systems

**Project Type:** Academic Engineering Project

**Developed Using:** Python, C, MySQL, HTML, CSS and JavaScript

---

⭐ If you find this project interesting, feel free to explore the repository!

**GasGrid – Making LPG Management Smarter, Simpler, and More Efficient.**
