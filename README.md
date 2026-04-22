# NexusDB

> A professional, production-ready C++17 terminal database engine backed by CSV files.

NexusDB provides a SQL-like interactive CLI for creating, querying, and managing
data tables stored as `.csv` files on disk. It features ANSI-colored output,
Unicode box-drawing table rendering, primary-key enforcement, and automatic
table discovery via `<filesystem>`.

---

## Features

- **SQL-like CLI** — Familiar command syntax (`CREATE TABLE`, `INSERT INTO`, `SELECT`, etc.)
- **CSV-backed storage** — Human-readable, portable data files
- **Auto-discovery** — Automatically loads all `.csv` tables from the `data/` directory
- **Primary Key enforcement** — INSERT, UPDATE, and DELETE operations validate PK constraints
- **ANSI-colored output** — Errors in red, success in green, headers in bold cyan
- **Box-drawing grids** — Query results displayed with Unicode table borders
- **Context-aware navigation** — `nexusdb > table_name > ` prompt shows your current context
- **Type validation** — INT, TEXT, and FLOAT types enforced on insert/update
- **Step-by-step insert wizard** — Guided data entry for each column

---

## Build Instructions

### Prerequisites
- A C++17-compatible compiler (GCC 8+, Clang 7+, MSVC 2017+)
- CMake 3.16 or higher

### Build

```bash
# Clone or navigate to the project directory
cd nexusDB

# Create a build directory
mkdir build && cd build

# Configure and build
cmake ..
cmake --build .
```

### Run

```bash
# From the build directory
./nexusdb        # Linux/macOS
nexusdb.exe      # Windows
```

The application will create a `data/` directory in the current working directory
if one does not exist.

---

## Command Reference

### Root Context (`nexusdb >`)

| Command | Description |
|---|---|
| `SHOW TABLES` | List all discovered tables |
| `CREATE TABLE <name> <col1:type, col2:type, ...>` | Create a new table (first column = PK) |
| `USE <tablename>` | Enter a table's context |
| `DROP TABLE <name>` | Delete a table and its CSV file from disk |
| `CLEAR` | Clear the terminal screen |
| `HELP` | Display available commands |
| `EXIT` | Quit NexusDB |

### Table Context (`nexusdb > tablename >`)

| Command | Description |
|---|---|
| `INSERT INTO <table>` | Launch step-by-step data entry wizard |
| `SELECT * FROM <table>` | Display all rows in a formatted grid |
| `SELECT * FROM <table> WHERE <col> = <val>` | Filter rows by column value |
| `UPDATE <table> SET <col>=<val>,... WHERE <pk>=<val>` | Update a row identified by PK |
| `DELETE FROM <table> WHERE <pk>=<val>` | Delete a row identified by PK |
| `DESCRIBE` | Show the table's column schema |
| `BACK` | Return to root context |
| `HELP` | Display table-context commands |

### Column Types

| Type | Description | Example |
|---|---|---|
| `INT` | Integer values | `42`, `-7` |
| `TEXT` | Any string value | `Alice`, `hello world` |
| `FLOAT` | Decimal numbers | `3.14`, `-0.5` |

---

## Example Session

```
nexusdb > CREATE TABLE users id:INT, name:TEXT, email:TEXT
  [OK] Table 'users' created successfully.

nexusdb > USE users
  [OK] Switched to table 'users'.

nexusdb > users > INSERT INTO users
  Insert into 'users' — enter values for each column:
  id (INT, PK): 1
  name (TEXT): Alice
  email (TEXT): alice@example.com
  [OK] Row inserted successfully.

nexusdb > users > INSERT INTO users
  id (INT, PK): 2
  name (TEXT): Bob
  email (TEXT): bob@example.com
  [OK] Row inserted successfully.

nexusdb > users > SELECT * FROM users
  ┌─────┬───────┬──────────────────┐
  │ id  │ name  │ email            │
  ├─────┼───────┼──────────────────┤
  │ 1   │ Alice │ alice@example.com│
  │ 2   │ Bob   │ bob@example.com  │
  └─────┴───────┴──────────────────┘
  2 row(s) returned.

nexusdb > users > UPDATE users SET name=Charlie WHERE id = 2
  [OK] Row updated successfully.

nexusdb > users > DELETE FROM users WHERE id = 1
  [OK] Row deleted successfully.

nexusdb > users > BACK

nexusdb > DROP TABLE users
  Are you sure? (yes/no): yes
  [OK] Table 'users' dropped.

nexusdb > EXIT
  Goodbye!
```

---

## Architecture

```
src/
├── ANSIColors.h        ANSI escape code constants
├── Column.h            Column struct & ColumnType enum
├── Row.h / Row.cpp     Row class (map-based record storage)
├── Table.h / Table.cpp Table class (schema, CRUD, CSV I/O)
├── NexusManager.h/cpp  Singleton manager (table registry)
├── CommandParser.h/cpp Command tokenizer & dispatcher
├── GridFormatter.h     Box-drawing grid renderer
└── main.cpp            Entry point & REPL loop
```

### Data Storage

Tables are stored as `.csv` files in the `data/` directory. The first line of
each file defines the schema:

```
id:INT:pk,name:TEXT,age:INT
1,Alice,30
2,Bob,25
```

---

## License

This project is provided as-is for educational and professional use.
