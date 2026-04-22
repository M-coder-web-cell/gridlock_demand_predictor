/**
 * @file CommandParser.h
 * @brief Declares the CommandParser — tokenizes user input and dispatches
 *        commands to the NexusManager / Table methods.
 */

#ifndef NEXUSDB_COMMANDPARSER_H
#define NEXUSDB_COMMANDPARSER_H

#include <string>
#include <vector>

namespace nexusdb {

/**
 * @class CommandParser
 * @brief Parses raw CLI input and executes the corresponding NexusDB command.
 *
 * The parser is context-aware:
 *   - Root context:  SHOW TABLES, CREATE TABLE, DROP TABLE, USE, HELP, EXIT
 *   - Table context: INSERT INTO, SELECT, UPDATE, DELETE, DESCRIBE, HELP, BACK
 */
class CommandParser {
public:
    /**
     * @brief Parse and execute a single command string.
     * @param input  Raw user input from the REPL.
     * @return false if the user requested EXIT, true otherwise.
     */
    static bool execute(const std::string& input);

private:
    // ── Tokenization helpers ─────────────────────────────────────
    static std::vector<std::string> tokenize(const std::string& input);
    static std::string toUpper(const std::string& s);
    static std::string trim(const std::string& s);

    // ── Root-context commands ────────────────────────────────────
    static void cmdShowTables();
    static void cmdCreateTable(const std::vector<std::string>& tokens);
    static void cmdDropTable(const std::vector<std::string>& tokens);
    static void cmdUseTable(const std::vector<std::string>& tokens);
    static void cmdHelpRoot();

    // ── Table-context commands ───────────────────────────────────
    static void cmdInsertInto(const std::vector<std::string>& tokens);
    static void cmdSelect(const std::vector<std::string>& tokens,
                          const std::string& rawInput);
    static void cmdUpdate(const std::vector<std::string>& tokens,
                          const std::string& rawInput);
    static void cmdDelete(const std::vector<std::string>& tokens,
                          const std::string& rawInput);
    static void cmdDescribe();
    static void cmdHelpTable();

    // ── Output helpers ───────────────────────────────────────────
    static void printError(const std::string& msg);
    static void printSuccess(const std::string& msg);
    static void printInfo(const std::string& msg);
};

} // namespace nexusdb

#endif // NEXUSDB_COMMANDPARSER_H
