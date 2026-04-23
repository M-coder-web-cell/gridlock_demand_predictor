"""
NexusDB REST API Server
Exposes NexusDB's CSV-backed engine over HTTP so any UI can interact with it.

Endpoints:
  GET    /api/tables                          - List all tables
  POST   /api/tables                          - Create a table
  DELETE /api/tables/<name>                   - Drop a table
  GET    /api/tables/<name>/schema            - Describe a table
  GET    /api/tables/<name>/rows              - SELECT * (optional ?where=col:val)
  POST   /api/tables/<name>/rows             - INSERT a row
  PUT    /api/tables/<name>/rows/<pk>        - UPDATE a row
  DELETE /api/tables/<name>/rows/<pk>        - DELETE a row
"""

import os
import csv
import json
import re
from pathlib import Path
from flask import Flask, request, jsonify
from flask_cors import CORS

# ─── Configuration ────────────────────────────────────────────────────────────
DATA_DIR = Path(__file__).parent.parent / "build" / "data"
DATA_DIR.mkdir(exist_ok=True)

app = Flask(__name__)
CORS(app)


# ─── CSV Engine ───────────────────────────────────────────────────────────────

def _csv_path(table_name: str) -> Path:
    return DATA_DIR / f"{table_name}.csv"


def _table_exists(name: str) -> bool:
    return _csv_path(name).exists()


def _list_tables() -> list[str]:
    return sorted(p.stem for p in DATA_DIR.glob("*.csv"))


def _parse_schema_line(header: str) -> list[dict]:
    """Parse 'id:INT:pk,name:TEXT,price:FLOAT' into column dicts."""
    cols = []
    for token in header.strip().split(","):
        token = token.strip()
        parts = token.split(":")
        if len(parts) < 2:
            raise ValueError(f"Invalid column token: {token!r}")
        col = {
            "name": parts[0].strip(),
            "type": parts[1].strip().upper(),
            "isPrimaryKey": len(parts) >= 3 and parts[2].strip().lower() == "pk"
        }
        if col["type"] not in ("INT", "TEXT", "FLOAT"):
            raise ValueError(f"Unknown type: {col['type']!r}")
        cols.append(col)
    return cols


def _schema_line(cols: list[dict]) -> str:
    tokens = []
    for c in cols:
        t = f"{c['name']}:{c['type']}"
        if c.get("isPrimaryKey"):
            t += ":pk"
        tokens.append(t)
    return ",".join(tokens)


def _read_table(name: str) -> tuple[list[dict], list[dict]]:
    """Returns (schema, rows) for a table."""
    path = _csv_path(name)
    if not path.exists():
        raise FileNotFoundError(f"Table '{name}' not found.")
    with open(path, newline="", encoding="utf-8") as f:
        reader = csv.reader(f)
        lines = list(reader)
    if not lines:
        return [], []
    schema = _parse_schema_line(",".join(lines[0]))
    col_names = [c["name"] for c in schema]
    rows = []
    for line in lines[1:]:
        if line:
            rows.append(dict(zip(col_names, line)))
    return schema, rows


def _write_table(name: str, schema: list[dict], rows: list[dict]):
    path = _csv_path(name)
    col_names = [c["name"] for c in schema]
    with open(path, "w", newline="", encoding="utf-8") as f:
        writer = csv.writer(f)
        writer.writerow([_schema_line([c]) for c in schema])
        for row in rows:
            writer.writerow([row.get(cn, "") for cn in col_names])


def _validate(value: str, col_type: str) -> bool:
    if col_type == "INT":
        try:
            int(value)
            return True
        except ValueError:
            return False
    if col_type == "FLOAT":
        try:
            float(value)
            return True
        except ValueError:
            return False
    return True  # TEXT accepts anything


def _pk_col(schema: list[dict]) -> dict | None:
    for c in schema:
        if c.get("isPrimaryKey"):
            return c
    return None


# ─── Routes ──────────────────────────────────────────────────────────────────

@app.route("/api/tables", methods=["GET"])
def list_tables():
    tables = []
    for name in _list_tables():
        try:
            schema, rows = _read_table(name)
            pk = _pk_col(schema)
            tables.append({
                "name": name,
                "columns": len(schema),
                "rows": len(rows),
                "primaryKey": pk["name"] if pk else None
            })
        except Exception as e:
            tables.append({"name": name, "error": str(e)})
    return jsonify({"tables": tables})


@app.route("/api/tables", methods=["POST"])
def create_table():
    """
    Body: { "name": "users", "columns": [{"name":"id","type":"INT","isPrimaryKey":true}, ...] }
    """
    body = request.get_json(force=True) or {}
    name = (body.get("name") or "").strip()
    cols = body.get("columns", [])

    if not name:
        return jsonify({"error": "Table name is required."}), 400
    if not re.match(r'^[A-Za-z_][A-Za-z0-9_]*$', name):
        return jsonify({"error": "Table name must be alphanumeric/underscore."}), 400
    if _table_exists(name):
        return jsonify({"error": f"Table '{name}' already exists."}), 409
    if not cols:
        return jsonify({"error": "At least one column is required."}), 400

    # Validate column defs
    seen = set()
    pk_count = 0
    for i, c in enumerate(cols):
        n = (c.get("name") or "").strip()
        t = (c.get("type") or "").upper().strip()
        if not n:
            return jsonify({"error": f"Column {i+1} has no name."}), 400
        if t not in ("INT", "TEXT", "FLOAT"):
            return jsonify({"error": f"Column '{n}' has invalid type '{t}'."}), 400
        if n in seen:
            return jsonify({"error": f"Duplicate column name '{n}'."}), 400
        seen.add(n)
        if c.get("isPrimaryKey"):
            pk_count += 1

    # First column is PK if none specified
    if pk_count == 0:
        cols[0]["isPrimaryKey"] = True

    schema = [{"name": c["name"].strip(), "type": c["type"].upper().strip(),
               "isPrimaryKey": bool(c.get("isPrimaryKey"))} for c in cols]

    _write_table(name, schema, [])
    return jsonify({"message": f"Table '{name}' created.", "schema": schema}), 201


@app.route("/api/tables/<name>", methods=["DELETE"])
def drop_table(name):
    if not _table_exists(name):
        return jsonify({"error": f"Table '{name}' not found."}), 404
    _csv_path(name).unlink()
    return jsonify({"message": f"Table '{name}' dropped."})


@app.route("/api/tables/<name>/schema", methods=["GET"])
def describe_table(name):
    try:
        schema, rows = _read_table(name)
        return jsonify({"table": name, "schema": schema, "rowCount": len(rows)})
    except FileNotFoundError as e:
        return jsonify({"error": str(e)}), 404


@app.route("/api/tables/<name>/rows", methods=["GET"])
def select_rows(name):
    """Optional ?where=<col>:<value>"""
    try:
        schema, rows = _read_table(name)
    except FileNotFoundError as e:
        return jsonify({"error": str(e)}), 404

    where = request.args.get("where")
    if where:
        parts = where.split(":", 1)
        if len(parts) == 2:
            col, val = parts
            rows = [r for r in rows if r.get(col) == val]

    return jsonify({"table": name, "schema": schema, "rows": rows, "count": len(rows)})


@app.route("/api/tables/<name>/rows", methods=["POST"])
def insert_row(name):
    """Body: { "row": { "id": "1", "name": "Alice", ... } }"""
    try:
        schema, rows = _read_table(name)
    except FileNotFoundError as e:
        return jsonify({"error": str(e)}), 404

    body = request.get_json(force=True) or {}
    row_data = body.get("row", {})

    # Validate all columns present and types correct
    col_names = [c["name"] for c in schema]
    for col in schema:
        val = str(row_data.get(col["name"], "")).strip()
        if not val and col.get("isPrimaryKey"):
            return jsonify({"error": f"Primary key '{col['name']}' cannot be empty."}), 400
        if val and not _validate(val, col["type"]):
            return jsonify({"error": f"Invalid {col['type']} value '{val}' for column '{col['name']}'."}), 400

    # PK uniqueness check
    pk = _pk_col(schema)
    if pk:
        pk_val = str(row_data.get(pk["name"], "")).strip()
        if any(r.get(pk["name"]) == pk_val for r in rows):
            return jsonify({"error": f"Duplicate primary key '{pk_val}' in column '{pk['name']}'."}), 409

    new_row = {c["name"]: str(row_data.get(c["name"], "")).strip() for c in schema}
    rows.append(new_row)
    _write_table(name, schema, rows)
    return jsonify({"message": "Row inserted.", "row": new_row}), 201


@app.route("/api/tables/<name>/rows/<pk_val>", methods=["PUT"])
def update_row(name, pk_val):
    """Body: { "updates": { "col": "newval", ... } }"""
    try:
        schema, rows = _read_table(name)
    except FileNotFoundError as e:
        return jsonify({"error": str(e)}), 404

    pk = _pk_col(schema)
    if not pk:
        return jsonify({"error": "No primary key defined for this table."}), 400

    body = request.get_json(force=True) or {}
    updates = body.get("updates", {})

    idx = next((i for i, r in enumerate(rows) if r.get(pk["name"]) == pk_val), -1)
    if idx == -1:
        return jsonify({"error": f"No row found with {pk['name']} = '{pk_val}'."}), 404

    for col_name, val in updates.items():
        if col_name == pk["name"]:
            return jsonify({"error": f"Cannot update primary key column '{pk['name']}'."}), 400
        col_def = next((c for c in schema if c["name"] == col_name), None)
        if not col_def:
            return jsonify({"error": f"Unknown column '{col_name}'."}), 400
        if not _validate(str(val), col_def["type"]):
            return jsonify({"error": f"Invalid {col_def['type']} value '{val}' for '{col_name}'."}), 400
        rows[idx][col_name] = str(val)

    _write_table(name, schema, rows)
    return jsonify({"message": "Row updated.", "row": rows[idx]})


@app.route("/api/tables/<name>/rows/<pk_val>", methods=["DELETE"])
def delete_row(name, pk_val):
    try:
        schema, rows = _read_table(name)
    except FileNotFoundError as e:
        return jsonify({"error": str(e)}), 404

    pk = _pk_col(schema)
    if not pk:
        return jsonify({"error": "No primary key defined."}), 400

    new_rows = [r for r in rows if r.get(pk["name"]) != pk_val]
    if len(new_rows) == len(rows):
        return jsonify({"error": f"No row found with {pk['name']} = '{pk_val}'."}), 404

    _write_table(name, schema, new_rows)
    return jsonify({"message": "Row deleted."})


@app.route("/api/health", methods=["GET"])
def health():
    return jsonify({"status": "ok", "dataDir": str(DATA_DIR), "tables": len(_list_tables())})


if __name__ == "__main__":
    print(f"[NexusDB API] Data dir: {DATA_DIR}")
    print("[NexusDB API] Listening on http://localhost:5000")
    app.run(debug=True, port=5000, use_reloader=False)