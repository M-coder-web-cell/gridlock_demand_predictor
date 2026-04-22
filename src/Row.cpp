/**
 * @file Row.cpp
 * @brief Implements the Row class methods.
 */

#include "Row.h"
#include <sstream>

namespace nexusdb {

std::string Row::get(const std::string& columnName) const {
    auto it = data_.find(columnName);
    if (it != data_.end()) return it->second;
    return "";
}

void Row::set(const std::string& columnName, const std::string& value) {
    data_[columnName] = value;
}

bool Row::has(const std::string& columnName) const {
    return data_.find(columnName) != data_.end();
}

/**
 * Produces a CSV line by iterating over the schema in order.
 * Values containing commas or quotes are wrapped in double-quotes.
 */
std::string Row::toCSVLine(const std::vector<Column>& schema) const {
    std::string line;
    for (size_t i = 0; i < schema.size(); ++i) {
        if (i > 0) line += ",";
        std::string val = get(schema[i].name);
        // Escape if the value contains comma, quote, or newline
        if (val.find(',') != std::string::npos ||
            val.find('"') != std::string::npos ||
            val.find('\n') != std::string::npos) {
            std::string escaped;
            escaped += '"';
            for (char c : val) {
                if (c == '"') escaped += "\"\"";
                else escaped += c;
            }
            escaped += '"';
            val = escaped;
        }
        line += val;
    }
    return line;
}

/**
 * Parses a CSV line respecting quoted fields.
 */
void Row::fromCSVLine(const std::string& line, const std::vector<Column>& schema) {
    data_.clear();
    std::vector<std::string> fields;

    bool inQuotes = false;
    std::string current;
    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (inQuotes) {
            if (c == '"') {
                // Check for escaped quote
                if (i + 1 < line.size() && line[i + 1] == '"') {
                    current += '"';
                    ++i; // skip next quote
                } else {
                    inQuotes = false;
                }
            } else {
                current += c;
            }
        } else {
            if (c == '"') {
                inQuotes = true;
            } else if (c == ',') {
                fields.push_back(current);
                current.clear();
            } else {
                current += c;
            }
        }
    }
    fields.push_back(current); // last field

    for (size_t i = 0; i < schema.size() && i < fields.size(); ++i) {
        data_[schema[i].name] = fields[i];
    }
}

const std::map<std::string, std::string>& Row::getData() const {
    return data_;
}

} // namespace nexusdb
