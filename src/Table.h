/**
 * @file Table.h
 * @brief Declares the Table class — the central data structure of NexusDB.
 *
 * A Table owns a schema (vector of Columns) and a collection of Rows.
 * It is responsible for CSV persistence and for enforcing primary-key
 * constraints on INSERT, UPDATE, and DELETE operations.
 */

#ifndef NEXUSDB_TABLE_H
#define NEXUSDB_TABLE_H

#include <string>
#include <vector>
#include <map>
#include "Column.h"
#include "Row.h"

namespace nexusdb {

/**
 * @class Table
 * @brief Manages schema, data, and CSV I/O for a single table.
 */
class Table {
public:
    /**
     * @brief Construct an empty table with a given name and schema.
     */
    Table(const std::string& name, const std::vector<Column>& schema);

    /**
     * @brief Construct a table by loading from a CSV file path.
     */
    explicit Table(const std::string& csvPath);

    // ── Accessors ────────────────────────────────────────────────
    const std::string&         getName()     const;
    const std::vector<Column>& getColumns()  const;
    const std::vector<Row>&    getRows()     const;
    const Column*              getPrimaryKey() const;

    // ── CSV I/O ──────────────────────────────────────────────────
    void loadFromCSV(const std::string& path);
    void saveToCSV(const std::string& path) const;
    void save() const;
    void setFilePath(const std::string& path);
    const std::string& getFilePath() const;

    // ── CRUD Operations ──────────────────────────────────────────
    bool insertRow(const Row& row, std::string& errMsg);
    std::vector<Row> selectAll() const;
    std::vector<Row> selectWhere(const std::string& pkValue) const;
    bool updateRow(const std::string& pkValue,
                   const std::map<std::string, std::string>& updates,
                   std::string& errMsg);
    bool deleteRow(const std::string& pkValue, std::string& errMsg);

    /** @brief Validate a value against a column's declared type. */
    static bool validateType(const std::string& value, ColumnType type);

private:
    std::string          name_;
    std::vector<Column>  schema_;
    std::vector<Row>     rows_;
    std::string          filePath_;

    int findRowByPK(const std::string& pkValue) const;
};

} // namespace nexusdb

#endif // NEXUSDB_TABLE_H
