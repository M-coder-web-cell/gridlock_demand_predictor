# NexusDB — Command Reference & Examples

---

## Supported Data Types

| Type    | Description              | Valid Examples              | Invalid Examples   |
|---------|--------------------------|-----------------------------|--------------------|
| `INT`   | Whole numbers            | `1`, `42`, `-7`, `+100`    | `3.14`, `abc`      |
| `TEXT`  | Any string value         | `Alice`, `hello world`     | *(anything is valid)* |
| `FLOAT` | Decimal numbers          | `3.14`, `-0.5`, `100`, `+2.7` | `abc`, `1.2.3`  |

---

## How to Build & Run

```powershell
# Navigate to the project
cd c:\Users\Acer\OneDrive\Desktop\nexusDB

# Compile (already done)
C:\MinGW\bin\g++.exe -std=c++14 -Wno-unused-variable -o build\nexusdb.exe ^
    src\main.cpp src\Row.cpp src\Table.cpp src\NexusManager.cpp src\CommandParser.cpp -Isrc

# Run
cd build
.\nexusdb.exe
```

---

## Complete Command Examples

### ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
### ROOT CONTEXT (prompt: `nexusdb >`)
### ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

### 1. HELP
Shows all available commands for the current context.
```
nexusdb > HELP
```

---

### 2. CREATE TABLE
Creates a new table. The **first column automatically becomes the Primary Key**.

**Syntax:**
```
CREATE TABLE <name> <col1:type, col2:type, ...>
```

**Examples:**
```
nexusdb > CREATE TABLE users id:INT, name:TEXT, email:TEXT
nexusdb > CREATE TABLE products pid:INT, title:TEXT, price:FLOAT
nexusdb > CREATE TABLE scores student_id:INT, subject:TEXT, marks:INT, percentage:FLOAT
```

---

### 3. SHOW TABLES
Lists all tables that NexusDB has discovered in the `data/` folder.
```
nexusdb > SHOW TABLES
```

**Output:**
```
  Tables
  ────────────────────────
   ▸ users
   ▸ products
   ▸ scores

  3 table(s) found.
```

---

### 4. USE
Enter a table's context to run queries on it.

**Syntax:**
```
USE <tablename>
```

**Example:**
```
nexusdb > USE users
  [OK] Switched to table 'users'.

nexusdb > users >
```

---

### 5. DROP TABLE
Permanently deletes a table and its `.csv` file. Asks for confirmation.

**Syntax:**
```
DROP TABLE <name>
```

**Example:**
```
nexusdb > DROP TABLE products
  Are you sure you want to drop table 'products'? This will delete the data file. (yes/no): yes
  [OK] Table 'products' dropped.
```

---

### 6. CLEAR
Clears the terminal screen.
```
nexusdb > CLEAR
```

---

### 7. EXIT
Quits NexusDB.
```
nexusdb > EXIT
  Goodbye!
```

---

### ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
### TABLE CONTEXT (prompt: `nexusdb > users >`)
### ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

> First run `USE users` to enter the table context.

---

### 8. DESCRIBE
Shows the table's column schema with types and primary key info.
```
nexusdb > users > DESCRIBE
```

**Output:**
```
  Schema for 'users'
  ────────────────────────────────────
   id : INT [PK]
   name : TEXT
   email : TEXT
```

---

### 9. INSERT INTO
Launches a step-by-step wizard — prompts you for each column value.

**Syntax:**
```
INSERT INTO <table>
```

**Example — Adding 3 rows:**
```
nexusdb > users > INSERT INTO users
  Insert into 'users' — enter values for each column:
  id (INT, PK): 1
  name (TEXT): Alice
  email (TEXT): alice@gmail.com
  [OK] Row inserted successfully.

nexusdb > users > INSERT INTO users
  id (INT, PK): 2
  name (TEXT): Bob
  email (TEXT): bob@yahoo.com
  [OK] Row inserted successfully.

nexusdb > users > INSERT INTO users
  id (INT, PK): 3
  name (TEXT): Charlie
  email (TEXT): charlie@outlook.com
  [OK] Row inserted successfully.
```

**Error examples:**
```
nexusdb > users > INSERT INTO users
  id (INT, PK): 1
  [ERROR] Duplicate primary key '1' in column 'id'.

nexusdb > users > INSERT INTO users
  id (INT, PK): abc
  [ERROR] Invalid INT value for 'id'. Insert aborted.
```

---

### 10. SELECT * FROM — View All Rows
Displays all rows in a formatted grid.

**Syntax:**
```
SELECT * FROM <table>
```

**Example:**
```
nexusdb > users > SELECT * FROM users
```

**Output:**
```
  ┌─────┬─────────┬─────────────────────┐
  │ id  │ name    │ email               │
  ├─────┼─────────┼─────────────────────┤
  │ 1   │ Alice   │ alice@gmail.com     │
  │ 2   │ Bob     │ bob@yahoo.com       │
  │ 3   │ Charlie │ charlie@outlook.com │
  └─────┴─────────┴─────────────────────┘
  3 row(s) returned.
```

---

### 11. SELECT with WHERE — Filter Rows
Filter results by a specific column value.

**Syntax:**
```
SELECT * FROM <table> WHERE <column> = <value>
```

**Examples:**
```
nexusdb > users > SELECT * FROM users WHERE id = 2
```

**Output:**
```
  ┌─────┬──────┬───────────────┐
  │ id  │ name │ email         │
  ├─────┼──────┼───────────────┤
  │ 2   │ Bob  │ bob@yahoo.com │
  └─────┴──────┴───────────────┘
  1 row(s) returned.
```

---

### 12. UPDATE — Modify a Row by Primary Key
Update one or more columns of a specific row.

**Syntax:**
```
UPDATE <table> SET <col>=<value>, <col2>=<value2> WHERE <pk_col> = <pk_value>
```

**Examples:**
```
nexusdb > users > UPDATE users SET name=Bobby WHERE id = 2
  [OK] Row updated successfully.

nexusdb > users > UPDATE users SET name=Alice Smith, email=alice.smith@gmail.com WHERE id = 1
  [OK] Row updated successfully.
```

**Error examples:**
```
nexusdb > users > UPDATE users SET name=Test WHERE id = 999
  [ERROR] No row found with id = '999'.

nexusdb > users > UPDATE users SET id=10 WHERE id = 1
  [ERROR] Cannot update primary key column 'id'.
```

---

### 13. DELETE FROM — Remove a Row by Primary Key

**Syntax:**
```
DELETE FROM <table> WHERE <pk_col> = <pk_value>
```

**Example:**
```
nexusdb > users > DELETE FROM users WHERE id = 3
  [OK] Row deleted successfully.
```

**Error example:**
```
nexusdb > users > DELETE FROM users WHERE id = 999
  [ERROR] No row found with id = '999'.
```

---

### 14. BACK
Return to the root context.
```
nexusdb > users > BACK
nexusdb >
```

---

### 15. HELP (Table Context)
Shows commands available inside the current table.
```
nexusdb > users > HELP
```

---

## Full Example Session (Copy-Paste Ready)

```
CREATE TABLE employees id:INT, name:TEXT, department:TEXT, salary:FLOAT
USE employees
INSERT INTO employees
  → id: 1
  → name: John Doe
  → department: Engineering
  → salary: 85000.50
INSERT INTO employees
  → id: 2
  → name: Jane Smith
  → department: Marketing
  → salary: 72000.00
INSERT INTO employees
  → id: 3
  → name: Mike Wilson
  → department: Engineering
  → salary: 91000.75
SELECT * FROM employees
SELECT * FROM employees WHERE id = 2
UPDATE employees SET salary=95000.00 WHERE id = 3
DELETE FROM employees WHERE id = 1
SELECT * FROM employees
DESCRIBE
BACK
SHOW TABLES
DROP TABLE employees
EXIT
```

---

## Notes

- **Primary Key** is always the **first column** defined in CREATE TABLE.
- **PK values must be unique** — duplicate inserts are rejected.
- **UPDATE and DELETE require WHERE with the Primary Key** — no bulk operations.
- All data is saved to `.csv` files in the `data/` folder automatically.
- Tables are auto-discovered from `data/` when NexusDB starts.
