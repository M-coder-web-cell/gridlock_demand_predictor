/**
 * @file GridFormatter.h
 * @brief Renders query results as a formatted table using Unicode box-drawing
 *        characters, with ANSI-colored headers.
 *
 * Example output:
 *   ┌──────┬────────────┬─────┐
 *   │ id   │ name       │ age │
 *   ├──────┼────────────┼─────┤
 *   │ 1    │ Alice      │ 30  │
 *   │ 2    │ Bob        │ 25  │
 *   └──────┴────────────┴─────┘
 */

#ifndef NEXUSDB_GRIDFORMATTER_H
#define NEXUSDB_GRIDFORMATTER_H

#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include "Column.h"
#include "Row.h"
#include "ANSIColors.h"

namespace nexusdb {

class GridFormatter {
public:
    /**
     * @brief Render rows as a box-drawing grid to stdout.
     * @param schema  The table columns (defines header names and order).
     * @param rows    The data rows to display.
     */
    static void render(const std::vector<Column>& schema,
                       const std::vector<Row>& rows) {
        if (schema.empty()) {
            std::cout << color::DIM_WHITE << "  (empty schema)" << color::RESET << "\n";
            return;
        }

        // ── 1. Compute column widths ─────────────────────────────────
        std::vector<size_t> widths(schema.size());
        for (size_t i = 0; i < schema.size(); ++i) {
            widths[i] = schema[i].name.size();
        }
        for (const auto& row : rows) {
            for (size_t i = 0; i < schema.size(); ++i) {
                size_t len = row.get(schema[i].name).size();
                if (len > widths[i]) widths[i] = len;
            }
        }
        // Add padding (minimum 2 extra chars)
        for (auto& w : widths) w = std::max(w, static_cast<size_t>(3));

        // ── 2. Build horizontal borders ──────────────────────────────
        auto hline = [&](const std::string& left,
                         const std::string& mid,
                         const std::string& right) -> std::string {
            std::string s = left;
            for (size_t i = 0; i < widths.size(); ++i) {
                for (size_t j = 0; j < widths[i] + 2; ++j) s += "\xe2\x94\x80"; // ─
                if (i < widths.size() - 1) s += mid;
            }
            s += right;
            return s;
        };

        std::string topBorder    = hline("\xe2\x94\x8c", "\xe2\x94\xac", "\xe2\x94\x90"); // ┌ ┬ ┐
        std::string midBorder    = hline("\xe2\x94\x9c", "\xe2\x94\xbc", "\xe2\x94\xa4"); // ├ ┼ ┤
        std::string bottomBorder = hline("\xe2\x94\x94", "\xe2\x94\xb4", "\xe2\x94\x98"); // └ ┴ ┘

        // ── 3. Print top border ──────────────────────────────────────
        std::cout << color::DIM_WHITE << "  " << topBorder << color::RESET << "\n";

        // ── 4. Print header row ──────────────────────────────────────
        std::cout << color::DIM_WHITE << "  \xe2\x94\x82" << color::RESET; // │
        for (size_t i = 0; i < schema.size(); ++i) {
            std::cout << " " << color::BOLD_CYAN;
            std::cout << std::left << std::setw(static_cast<int>(widths[i]))
                      << schema[i].name;
            std::cout << color::RESET << " ";
            std::cout << color::DIM_WHITE << "\xe2\x94\x82" << color::RESET; // │
        }
        std::cout << "\n";

        // ── 5. Print mid border ──────────────────────────────────────
        std::cout << color::DIM_WHITE << "  " << midBorder << color::RESET << "\n";

        // ── 6. Print data rows ───────────────────────────────────────
        if (rows.empty()) {
            // Show an empty-set message spanning the full width
            size_t totalWidth = 0;
            for (auto w : widths) totalWidth += w + 3; // +3 for " | "
            totalWidth -= 1;
            std::cout << color::DIM_WHITE << "  \xe2\x94\x82" << color::RESET;
            std::cout << color::DIM_WHITE;
            std::string msg = " (no rows) ";
            size_t pad = (totalWidth > msg.size()) ? totalWidth - msg.size() : 0;
            std::cout << msg << std::string(pad, ' ');
            std::cout << color::RESET;
            std::cout << color::DIM_WHITE << "\xe2\x94\x82" << color::RESET << "\n";
        } else {
            for (const auto& row : rows) {
                std::cout << color::DIM_WHITE << "  \xe2\x94\x82" << color::RESET; // │
                for (size_t i = 0; i < schema.size(); ++i) {
                    std::string val = row.get(schema[i].name);
                    std::cout << " " << std::left
                              << std::setw(static_cast<int>(widths[i])) << val
                              << " ";
                    std::cout << color::DIM_WHITE << "\xe2\x94\x82" << color::RESET; // │
                }
                std::cout << "\n";
            }
        }

        // ── 7. Print bottom border ───────────────────────────────────
        std::cout << color::DIM_WHITE << "  " << bottomBorder << color::RESET << "\n";

        // ── 8. Row count ─────────────────────────────────────────────
        std::cout << color::DIM_WHITE << "  " << rows.size() << " row(s) returned."
                  << color::RESET << "\n";
    }
};

} // namespace nexusdb

#endif // NEXUSDB_GRIDFORMATTER_H
