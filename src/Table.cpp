/**
 * @file Table.cpp
 * @brief Implements the Table class — CSV serialization and CRUD operations.
 */

#include "Table.h"
#include "FileSystem.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace nexusdb {

// ═══════════════════════════════════════════════════════════════
// Constructors
// ═══════════════════════════════════════════════════════════════

Table::Table(const std::string& name, const std::vector<Column>& schema)
    : name_(name), schema_(schema) {}

Table::Table(const std::string& csvPath)
    : filePath_(csvPath) {
    name_ = fs::stem(csvPath);
    loadFromCSV(csvPath);
}

// ═══════════════════════════════════════════════════════════════
// Accessors
// ═══════════════════════════════════════════════════════════════

const std::string&         Table::getName()     const { return name_; }
const std::vector<Column>& Table::getColumns()  const { return schema_; }
const std::vector<Row>&    Table::getRows()     const { return rows_; }

const Column* Table::getPrimaryKey() const {
    for (const auto& col : schema_) {
        if (col.isPrimaryKey) return &col;
    }
    return nullptr;
}

void Table::setFilePath(const std::string& path) { filePath_ = path; }
const std::string& Table::getFilePath() const { return filePath_; }

// ═══════════════════════════════════════════════════════════════
// CSV I/O
// ═══════════════════════════════════════════════════════════════

void Table::loadFromCSV(const std::string& path) {
    std::ifstream ifs(path);
    if (!ifs.is_open()) {
        throw std::runtime_error("Cannot open file: " + path);
    }

    filePath_ = path;
    schema_.clear();
    rows_.clear();

    std::string line;

    // ── Read schema line ─────────────────────────────────────
    if (!std::getline(ifs, line)) return;
    if (!line.empty() && line.back() == '\r') line.pop_back();

    {
        std::istringstream ss(line);
        std::string token;
        while (std::getline(ss, token, ',')) {
            token.erase(0, token.find_first_not_of(" \t"));
            token.erase(token.find_last_not_of(" \t") + 1);
            if (!token.empty()) {
                schema_.push_back(Column::deserialize(token));
            }
        }
    }

    // ── Read data rows ───────────────────────────────────────
    while (std::getline(ifs, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;
        Row row;
        row.fromCSVLine(line, schema_);
        rows_.push_back(std::move(row));
    }
}

void Table::saveToCSV(const std::string& path) const {
    std::ofstream ofs(path, std::ios::trunc);
    if (!ofs.is_open()) {
        throw std::runtime_error("Cannot write to file: " + path);
    }

    // ── Schema header ────────────────────────────────────────
    for (size_t i = 0; i < schema_.size(); ++i) {
        if (i > 0) ofs << ",";
        ofs << schema_[i].serialize();
    }
    ofs << "\n";

    // ── Data rows ────────────────────────────────────────────
    for (const auto& row : rows_) {
        ofs << row.toCSVLine(schema_) << "\n";
    }
}

void Table::save() const {
    if (!filePath_.empty()) {
        saveToCSV(filePath_);
    }
}

// ═══════════════════════════════════════════════════════════════
// CRUD Operations
// ═══════════════════════════════════════════════════════════════

bool Table::insertRow(const Row& row, std::string& errMsg) {
    for (const auto& col : schema_) {
        std::string val = row.get(col.name);
        if (val.empty()) {
            errMsg = "Missing value for column '" + col.name + "'.";
            return false;
        }
        if (!validateType(val, col.type)) {
            errMsg = "Type mismatch for column '" + col.name +
                     "' (expected " + columnTypeToString(col.type) + ").";
            return false;
        }
    }

    const Column* pk = getPrimaryKey();
    if (pk) {
        std::string pkVal = row.get(pk->name);
        if (findRowByPK(pkVal) >= 0) {
            errMsg = "Duplicate primary key '" + pkVal + "' in column '" + pk->name + "'.";
            return false;
        }
    }

    rows_.push_back(row);
    save();
    return true;
}

std::vector<Row> Table::selectAll() const { return rows_; }

std::vector<Row> Table::selectWhere(const std::string& pkValue) const {
    std::vector<Row> result;
    const Column* pk = getPrimaryKey();
    if (!pk) return result;
    for (const auto& row : rows_) {
        if (row.get(pk->name) == pkValue) result.push_back(row);
    }
    return result;
}

bool Table::updateRow(const std::string& pkValue,
                      const std::map<std::string, std::string>& updates,
                      std::string& errMsg) {
    const Column* pk = getPrimaryKey();
    if (!pk) { errMsg = "Table has no primary key defined."; return false; }

    int idx = findRowByPK(pkValue);
    if (idx < 0) {
        errMsg = "No row found with " + pk->name + " = '" + pkValue + "'.";
        return false;
    }

    for (const auto& kv : updates) {
        bool found = false;
        for (const auto& col : schema_) {
            if (col.name == kv.first) {
                found = true;
                if (col.isPrimaryKey) {
                    errMsg = "Cannot update primary key column '" + kv.first + "'.";
                    return false;
                }
                if (!validateType(kv.second, col.type)) {
                    errMsg = "Type mismatch for column '" + kv.first +
                             "' (expected " + columnTypeToString(col.type) + ").";
                    return false;
                }
                break;
            }
        }
        if (!found) {
            errMsg = "Column '" + kv.first + "' does not exist in table '" + name_ + "'.";
            return false;
        }
    }

    for (const auto& kv : updates) {
        rows_[static_cast<size_t>(idx)].set(kv.first, kv.second);
    }

    save();
    return true;
}

bool Table::deleteRow(const std::string& pkValue, std::string& errMsg) {
    const Column* pk = getPrimaryKey();
    if (!pk) { errMsg = "Table has no primary key defined."; return false; }

    int idx = findRowByPK(pkValue);
    if (idx < 0) {
        errMsg = "No row found with " + pk->name + " = '" + pkValue + "'.";
        return false;
    }

    rows_.erase(rows_.begin() + idx);
    save();
    return true;
}

// ═══════════════════════════════════════════════════════════════
// Helpers
// ═══════════════════════════════════════════════════════════════

bool Table::validateType(const std::string& value, ColumnType type) {
    if (value.empty()) return false;
    switch (type) {
        case ColumnType::INT: {
            size_t start = 0;
            if (value[0] == '-' || value[0] == '+') start = 1;
            if (start >= value.size()) return false;
            for (size_t i = start; i < value.size(); ++i) {
                if (!std::isdigit(static_cast<unsigned char>(value[i]))) return false;
            }
            return true;
        }
        case ColumnType::FLOAT: {
            bool dotSeen = false;
            size_t start = 0;
            if (value[0] == '-' || value[0] == '+') start = 1;
            if (start >= value.size()) return false;
            for (size_t i = start; i < value.size(); ++i) {
                char c = value[i];
                if (c == '.') { if (dotSeen) return false; dotSeen = true; }
                else if (!std::isdigit(static_cast<unsigned char>(c))) return false;
            }
            return true;
        }
        case ColumnType::TEXT: return true;
        default: return false;
    }
}

int Table::findRowByPK(const std::string& pkValue) const {
    const Column* pk = getPrimaryKey();
    if (!pk) return -1;
    for (size_t i = 0; i < rows_.size(); ++i) {
        if (rows_[i].get(pk->name) == pkValue) return static_cast<int>(i);
    }
    return -1;
}

} // namespace nexusdb
