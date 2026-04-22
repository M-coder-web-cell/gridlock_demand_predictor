/**
 * @file Row.h
 * @brief Declares the Row class for storing a single record's data.
 *
 * A Row maps column names to their string-encoded values.  Methods are
 * provided for CSV serialization and deserialization against a schema.
 */

#ifndef NEXUSDB_ROW_H
#define NEXUSDB_ROW_H

#include <string>
#include <map>
#include <vector>
#include "Column.h"

namespace nexusdb {

/**
 * @class Row
 * @brief Represents a single row of data in a table.
 */
class Row {
public:
    Row() = default;

    /**
     * @brief Get the value stored for a given column.
     * @param columnName  Name of the column.
     * @return The value as a string, or empty string if not present.
     */
    std::string get(const std::string& columnName) const;

    /**
     * @brief Set the value for a given column.
     * @param columnName  Name of the column.
     * @param value       The value to store.
     */
    void set(const std::string& columnName, const std::string& value);

    /**
     * @brief Check whether a value exists for the given column.
     */
    bool has(const std::string& columnName) const;

    /**
     * @brief Serialize the row to a CSV line based on the given schema ordering.
     * @param schema  Ordered vector of columns defining field order.
     * @return A comma-separated string of values.
     */
    std::string toCSVLine(const std::vector<Column>& schema) const;

    /**
     * @brief Deserialize a CSV line into this Row using the given schema.
     * @param line    Comma-separated values.
     * @param schema  Ordered vector of columns defining field order.
     */
    void fromCSVLine(const std::string& line, const std::vector<Column>& schema);

    /**
     * @brief Get a const reference to the underlying data map.
     */
    const std::map<std::string, std::string>& getData() const;

private:
    std::map<std::string, std::string> data_; ///< Column name → value.
};

} // namespace nexusdb

#endif // NEXUSDB_ROW_H
