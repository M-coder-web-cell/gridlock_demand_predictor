/**
 * @file main.cpp
 * @brief Entry point for NexusDB — initializes the manager and runs the REPL.
 */

#include <iostream>
#include <string>
#include "ANSIColors.h"
#include "FileSystem.h"
#include "NexusManager.h"
#include "CommandParser.h"

#ifdef _WIN32
#include <windows.h>
#endif

using namespace nexusdb;

/** @brief Enable ANSI escape processing on Windows 10+ terminals. */
static void enableANSI() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD mode = 0;
        if (GetConsoleMode(hOut, &mode)) {
            mode |= 0x0004; // ENABLE_VIRTUAL_TERMINAL_PROCESSING
            SetConsoleMode(hOut, mode);
        }
    }
    SetConsoleOutputCP(65001);
#endif
}

/** @brief Print the NexusDB startup banner. */
static void printBanner() {
    std::cout << "\n";
    std::cout << color::BOLD_CYAN;
    std::cout << "  ============================================\n";
    std::cout << "    _   _                     ____  ____\n";
    std::cout << "   | \\ | | _____  ___   _ ___|  _ \\| __ )\n";
    std::cout << "   |  \\| |/ _ \\ \\/ / | | / __| | | |  _ \\\n";
    std::cout << "   | |\\  |  __/>  <| |_| \\__ \\ |_| | |_) |\n";
    std::cout << "   |_| \\_|\\___/_/\\_\\\\__,_|___/____/|____/\n";
    std::cout << "  ============================================\n";
    std::cout << color::RESET;
    std::cout << color::DIM_WHITE << "  v1.0.0 | C++ Terminal Database Engine\n";
    std::cout << "  Type " << color::BOLD_YELLOW << "HELP" << color::DIM_WHITE
              << " for available commands.\n" << color::RESET;
    std::cout << "\n";
}

int main() {
    enableANSI();
    printBanner();

    // ── Determine data directory ─────────────────────────────
    std::string dataDir = fs::joinPath(fs::currentPath(), "data");

    auto& mgr = NexusManager::getInstance();
    mgr.initialize(dataDir);

    auto tables = mgr.showTables();
    if (!tables.empty()) {
        std::cout << color::DIM_WHITE << "  Discovered " << tables.size()
                  << " table(s) in " << dataDir << color::RESET << "\n\n";
    }

    // ── REPL loop ────────────────────────────────────────────
    std::string input;
    bool running = true;
    while (running) {
        std::cout << color::BOLD_GREEN << "nexusdb" << color::RESET;
        Table* current = mgr.getCurrentTable();
        if (current) {
            std::cout << color::DIM_WHITE << " > " << color::RESET
                      << color::BOLD_CYAN << current->getName() << color::RESET;
        }
        std::cout << color::DIM_WHITE << " > " << color::RESET;

        if (!std::getline(std::cin, input)) {
            std::cout << "\n";
            break;
        }
        running = CommandParser::execute(input);
    }

    std::cout << color::BOLD_CYAN << "\n  Goodbye!\n" << color::RESET;
    return 0;
}
