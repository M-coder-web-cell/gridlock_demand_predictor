/**
 * @file QueryEngine.h
 * @brief WHERE clause parser and evaluator with support for comparison
 *        operators (=, !=, >, <, >=, <=) and logical operators (AND, OR).
 *
 * Examples:
 *   WHERE age > 20
 *   WHERE grade >= 9.0 AND age < 23
 *   WHERE name = Alice OR name = Bob
 *   WHERE age >= 18 AND grade > 7.0 AND name != Charlie
 *
 * Operator precedence: AND binds tighter than OR.
 */

#ifndef NEXUSDB_QUERYENGINE_H
#define NEXUSDB_QUERYENGINE_H

#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <sstream>
#include "Column.h"
#include "Row.h"

namespace nexusdb {

/**
 * @struct Condition
 * @brief A single comparison: column OP value.
 */
struct Condition {
    std::string column; ///< Column name
    std::string op;     ///< Operator: =, !=, >, <, >=, <=
    std::string value;  ///< Value to compare against
};

/**
 * @enum LogicOp
 * @brief Logical connectors between conditions.
 */
enum class LogicOp { NONE, AND, OR };

/**
 * @struct WhereClause
 * @brief A parsed WHERE expression: conditions connected by AND/OR.
 */
struct WhereClause {
    std::vector<Condition> conditions;
    std::vector<LogicOp>   logicOps; ///< Size = conditions.size() - 1
    bool valid = false;              ///< Whether parsing succeeded
    std::string error;               ///< Error message if parsing failed
};

/**
 * @class QueryEngine
 * @brief Static methods to parse and evaluate WHERE clauses.
 */
class QueryEngine {
public:

    /**
     * @brief Parse a WHERE string into a WhereClause.
     * @param whereStr  Everything after "WHERE" (e.g. "age > 20 AND name = Alice").
     * @return Parsed WhereClause with valid=true on success.
     */
    static WhereClause parseWhere(const std::string& whereStr) {
        WhereClause clause;
        std::string input = trim(whereStr);
        if (input.empty()) {
            clause.error = "Empty WHERE clause.";
            return clause;
        }

        // Tokenize respecting quoted strings and multi-char operators
        std::vector<std::string> tokens = tokenize(input);
        if (tokens.empty()) {
            clause.error = "Could not parse WHERE clause.";
            return clause;
        }

        // Parse: condition [AND|OR condition]*
        // condition = column operator value
        size_t i = 0;
        while (i < tokens.size()) {
            // Expect: column operator value
            if (i + 2 >= tokens.size()) {
                clause.error = "Incomplete condition near '" +
                    (i < tokens.size() ? tokens[i] : "end") + "'.";
                return clause;
            }

            Condition cond;
            cond.column = tokens[i];
            cond.op     = tokens[i + 1];
            cond.value  = stripQuotes(tokens[i + 2]);
            i += 3;

            // Validate operator
            if (!isValidOp(cond.op)) {
                clause.error = "Invalid operator '" + cond.op +
                    "'. Use =, !=, >, <, >=, <=";
                return clause;
            }

            clause.conditions.push_back(cond);

            // Check for AND / OR
            if (i < tokens.size()) {
                std::string logic = toUpper(tokens[i]);
                if (logic == "AND") {
                    clause.logicOps.push_back(LogicOp::AND);
                    i++;
                } else if (logic == "OR") {
                    clause.logicOps.push_back(LogicOp::OR);
                    i++;
                } else {
                    clause.error = "Expected AND/OR but got '" + tokens[i] + "'.";
                    return clause;
                }
            }
        }

        clause.valid = true;
        return clause;
    }

    /**
     * @brief Evaluate a single row against a parsed WhereClause.
     * @param row     The row to test.
     * @param schema  The table schema (for type-aware comparison).
     * @param clause  The parsed WHERE clause.
     * @return true if the row satisfies the WHERE clause.
     */
    static bool evaluate(const Row& row,
                         const std::vector<Column>& schema,
                         const WhereClause& clause) {
        if (!clause.valid || clause.conditions.empty()) return false;

        // Evaluate each condition
        std::vector<bool> results;
        results.reserve(clause.conditions.size());
        for (const auto& cond : clause.conditions) {
            results.push_back(evalCondition(row, schema, cond));
        }

        // Apply logic with AND having higher precedence than OR.
        // Step 1: Resolve all ANDs first
        // Group by OR: split into AND-groups
        std::vector<std::vector<bool>> andGroups;
        andGroups.push_back({results[0]});

        for (size_t i = 0; i < clause.logicOps.size(); ++i) {
            if (clause.logicOps[i] == LogicOp::AND) {
                andGroups.back().push_back(results[i + 1]);
            } else { // OR
                andGroups.push_back({results[i + 1]});
            }
        }

        // Step 2: Each AND-group is true only if all its members are true
        // Step 3: The overall result is true if any AND-group is true (OR logic)
        for (const auto& group : andGroups) {
            bool allTrue = true;
            for (bool b : group) {
                if (!b) { allTrue = false; break; }
            }
            if (allTrue) return true;
        }

        return false;
    }

    /**
     * @brief Filter a set of rows by a WHERE clause.
     */
    static std::vector<Row> filter(const std::vector<Row>& rows,
                                   const std::vector<Column>& schema,
                                   const WhereClause& clause) {
        std::vector<Row> result;
        for (const auto& row : rows) {
            if (evaluate(row, schema, clause)) {
                result.push_back(row);
            }
        }
        return result;
    }

private:

    /**
     * @brief Evaluate a single condition against a row.
     */
    static bool evalCondition(const Row& row,
                              const std::vector<Column>& schema,
                              const Condition& cond) {
        std::string rowVal = row.get(cond.column);

        // Find the column type
        ColumnType type = ColumnType::TEXT;
        for (const auto& col : schema) {
            if (col.name == cond.column) {
                type = col.type;
                break;
            }
        }

        return compare(rowVal, cond.op, cond.value, type);
    }

    /**
     * @brief Compare two values using the given operator and type.
     */
    static bool compare(const std::string& lhs, const std::string& op,
                        const std::string& rhs, ColumnType type) {

        if (type == ColumnType::INT) {
            long l = std::atol(lhs.c_str());
            long r = std::atol(rhs.c_str());
            if (op == "=")  return l == r;
            if (op == "!=") return l != r;
            if (op == ">")  return l > r;
            if (op == "<")  return l < r;
            if (op == ">=") return l >= r;
            if (op == "<=") return l <= r;
        }
        else if (type == ColumnType::FLOAT) {
            double l = std::atof(lhs.c_str());
            double r = std::atof(rhs.c_str());
            if (op == "=")  return l == r;
            if (op == "!=") return l != r;
            if (op == ">")  return l > r;
            if (op == "<")  return l < r;
            if (op == ">=") return l >= r;
            if (op == "<=") return l <= r;
        }
        else { // TEXT — string comparison
            if (op == "=")  return lhs == rhs;
            if (op == "!=") return lhs != rhs;
            if (op == ">")  return lhs > rhs;
            if (op == "<")  return lhs < rhs;
            if (op == ">=") return lhs >= rhs;
            if (op == "<=") return lhs <= rhs;
        }

        return false;
    }

    static bool isValidOp(const std::string& op) {
        return op == "=" || op == "!=" || op == ">" ||
               op == "<" || op == ">=" || op == "<=";
    }

    /**
     * @brief Tokenize a WHERE string, keeping multi-char operators together
     *        and handling quoted values.
     */
    static std::vector<std::string> tokenize(const std::string& input) {
        std::vector<std::string> tokens;
        size_t i = 0;
        size_t len = input.size();

        while (i < len) {
            // Skip whitespace
            while (i < len && (input[i] == ' ' || input[i] == '\t')) i++;
            if (i >= len) break;

            // Quoted string
            if (input[i] == '\'' || input[i] == '"') {
                char quote = input[i];
                i++; // skip opening quote
                std::string tok;
                while (i < len && input[i] != quote) {
                    tok += input[i];
                    i++;
                }
                if (i < len) i++; // skip closing quote
                tokens.push_back("'" + tok + "'");
                continue;
            }

            // Multi-char operators: !=, >=, <=
            if (i + 1 < len) {
                std::string two = input.substr(i, 2);
                if (two == "!=" || two == ">=" || two == "<=") {
                    tokens.push_back(two);
                    i += 2;
                    continue;
                }
            }

            // Single-char operators: =, >, <
            if (input[i] == '=' || input[i] == '>' || input[i] == '<') {
                tokens.push_back(std::string(1, input[i]));
                i++;
                continue;
            }

            // Regular word
            std::string tok;
            while (i < len && input[i] != ' ' && input[i] != '\t' &&
                   input[i] != '=' && input[i] != '!' &&
                   input[i] != '>' && input[i] != '<') {
                tok += input[i];
                i++;
            }
            if (!tok.empty()) tokens.push_back(tok);
        }

        return tokens;
    }

    static std::string stripQuotes(const std::string& s) {
        if (s.size() >= 2) {
            if ((s.front() == '\'' && s.back() == '\'') ||
                (s.front() == '"'  && s.back() == '"')) {
                return s.substr(1, s.size() - 2);
            }
        }
        return s;
    }

    static std::string trim(const std::string& s) {
        size_t a = s.find_first_not_of(" \t\r\n");
        if (a == std::string::npos) return "";
        size_t b = s.find_last_not_of(" \t\r\n");
        return s.substr(a, b - a + 1);
    }

    static std::string toUpper(const std::string& s) {
        std::string u = s;
        std::transform(u.begin(), u.end(), u.begin(), ::toupper);
        return u;
    }
};

} // namespace nexusdb

#endif // NEXUSDB_QUERYENGINE_H
