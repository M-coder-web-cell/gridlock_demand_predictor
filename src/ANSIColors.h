/**
 * @file ANSIColors.h
 * @brief ANSI escape code constants for terminal color output.
 *
 * Provides color constants and a helper function for styling
 * NexusDB CLI output: errors (Red), success (Green), headers (Bold Cyan).
 */

#ifndef NEXUSDB_ANSICOLORS_H
#define NEXUSDB_ANSICOLORS_H

#include <string>

namespace nexusdb {
namespace color {

// ── Reset ────────────────────────────────────────────────────────────
static const char* RESET     = "\033[0m";

// ── Style modifiers ──────────────────────────────────────────────────
static const char* BOLD      = "\033[1m";
static const char* DIM       = "\033[2m";
static const char* UNDERLINE = "\033[4m";

// ── Foreground colors ────────────────────────────────────────────────
static const char* RED       = "\033[31m";
static const char* GREEN     = "\033[32m";
static const char* YELLOW    = "\033[33m";
static const char* BLUE      = "\033[34m";
static const char* MAGENTA   = "\033[35m";
static const char* CYAN      = "\033[36m";
static const char* WHITE     = "\033[37m";

// ── Compound styles ──────────────────────────────────────────────────
static const char* BOLD_CYAN   = "\033[1;36m";
static const char* BOLD_GREEN  = "\033[1;32m";
static const char* BOLD_RED    = "\033[1;31m";
static const char* BOLD_YELLOW = "\033[1;33m";
static const char* BOLD_WHITE  = "\033[1;37m";
static const char* DIM_WHITE   = "\033[2;37m";

/**
 * @brief Wraps text in the given ANSI color code, appending a reset.
 */
inline std::string colorize(const std::string& text, const char* code) {
    return std::string(code) + text + RESET;
}

} // namespace color
} // namespace nexusdb

#endif // NEXUSDB_ANSICOLORS_H
