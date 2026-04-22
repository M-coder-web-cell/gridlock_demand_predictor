/**
 * @file Column.h
 * @brief Defines the Column struct and ColumnType enum for table schemas.
 *
 * Each Column has a name, a data type (INT, TEXT, FLOAT), and a flag
 * indicating whether it serves as the table's primary key.
 */

#ifndef NEXUSDB_COLUMN_H
#define NEXUSDB_COLUMN_H

#include <string>
#include <stdexcept>
#include <algorithm>

namespace nexusdb {

/**
 * @enum ColumnType
 * @brief Supported column data types.
 */
enum class ColumnType {
    INT,
    TEXT,
    FLOAT
};

/**
 * @brief Convert a ColumnType enum to its string representation.
 */
inline std::string columnTypeToString(ColumnType type) {
    switch (type) {
        case ColumnType::INT:   return "INT";
        case ColumnType::TEXT:  return "TEXT";
        case ColumnType::FLOAT: return "FLOAT";
        default:                return "UNKNOWN";
    }
}

/**
 * @brief Parse a string into a ColumnType enum (case-insensitive).
 * @throws std::invalid_argument if the string is not a recognized type.
 */
inline std::string trimStr(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

inline ColumnType stringToColumnType(const std::string& str) {
    std::string upper = trimStr(str);
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
    if (upper == "INT")   return ColumnType::INT;
    if (upper == "TEXT")  return ColumnType::TEXT;
    if (upper == "FLOAT") return ColumnType::FLOAT;
    throw std::invalid_argument("Unknown column type: " + str);
}

/**
 * @struct Column
 * @brief Describes a single column in a table schema.
 */
struct Column {
    std::string name;         ///< Column name (e.g. "id", "username").
    ColumnType  type;         ///< Data type of the column.
    bool        isPrimaryKey; ///< Whether this column is the primary key.

    Column() : name(""), type(ColumnType::TEXT), isPrimaryKey(false) {}

    Column(const std::string& name, ColumnType type, bool isPK = false)
        : name(name), type(type), isPrimaryKey(isPK) {}

    /**
     * @brief Serialize to a CSV-header token.
     *        Format: "name:TYPE" or "name:TYPE:pk"
     */
    std::string serialize() const {
        std::string s = name + ":" + columnTypeToString(type);
        if (isPrimaryKey) s += ":pk";
        return s;
    }

    /**
     * @brief Deserialize from a CSV-header token.
     * @throws std::invalid_argument on bad format.
     */
    static Column deserialize(const std::string& token) {
        Column col;
        // Split by ':'
        size_t p1 = token.find(':');
        if (p1 == std::string::npos)
            throw std::invalid_argument("Invalid column token: " + token);

        col.name = trimStr(token.substr(0, p1));
        size_t p2 = token.find(':', p1 + 1);
        if (p2 == std::string::npos) {
            col.type = stringToColumnType(token.substr(p1 + 1));
            col.isPrimaryKey = false;
        } else {
            col.type = stringToColumnType(token.substr(p1 + 1, p2 - p1 - 1));
            std::string flag = token.substr(p2 + 1);
            std::transform(flag.begin(), flag.end(), flag.begin(), ::tolower);
            col.isPrimaryKey = (flag == "pk");
        }
        return col;
    }
};

} // namespace nexusdb

#endif // NEXUSDB_COLUMN_H
