/**
 * @file NexusManager.h
 * @brief Declares the NexusManager singleton — the central orchestrator of
 *        NexusDB that manages all tables and tracks the current table context.
 */

#ifndef NEXUSDB_NEXUSMANAGER_H
#define NEXUSDB_NEXUSMANAGER_H

#include <string>
#include <vector>
#include "Table.h"

namespace nexusdb {

/**
 * @class NexusManager
 * @brief Singleton that owns all Table objects and manages the "current table"
 *        state for the CLI context system.
 */
class NexusManager {
public:
    static NexusManager& getInstance();

    NexusManager(const NexusManager&)            = delete;
    NexusManager& operator=(const NexusManager&) = delete;

    void initialize(const std::string& dataDir);
    void discoverTables();
    std::vector<std::string> showTables() const;
    bool useTable(const std::string& name);
    void clearCurrentTable();
    Table* getCurrentTable();
    Table* findTable(const std::string& name);
    bool createTable(const std::string& name,
                     const std::vector<Column>& schema,
                     std::string& errMsg);
    bool dropTable(const std::string& name, std::string& errMsg);
    const std::string& getDataDir() const;

private:
    NexusManager() = default;

    std::vector<Table> tables_;
    Table*             currentTable_ = nullptr;
    std::string        dataDir_;
};

} // namespace nexusdb

#endif // NEXUSDB_NEXUSMANAGER_H
