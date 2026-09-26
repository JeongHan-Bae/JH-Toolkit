/**
 * @copyright
 * Copyright 2025 JeongHan-Bae &lt;mastropseudo\@gmail.com&gt;
 * <br>
 * Licensed under the Apache License, Version 2.0 (the "License"); <br>
 * you may not use this file except in compliance with the License.<br>
 * You may obtain a copy of the License at<br>
 * <br>
 *     http://www.apache.org/licenses/LICENSE-2.0<br>
 * <br>
 * Unless required by applicable law or agreed to in writing, software<br>
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.<br>
 * See the License for the specific language governing permissions and<br>
 * limitations under the License.<br>
 * <br>
 * Full license: <a href="https://github.com/JeongHan-Bae/JH-Toolkit?tab=Apache-2.0-1-ov-file#readme">GitHub</a>
 */
/**
 * @file table.hpp
 * @brief DataTable types for Cucumber step attachments.
 * @author JeongHan-Bae <a href="mailto:mastropseudo&#64;gmail.com">&lt;mastropseudo\@gmail.com&gt;</a>
 */

#pragma once

#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "jh/metax/expected.h"

namespace jh::test::cucumber {
    /// @brief Describes why a DataTable operation could not be completed.
    enum class table_error {
        /// @brief The table has no non-empty header row.
        missing_header,
        /// @brief A data row has a different number of cells than the header.
        row_size_mismatch,
        /// @brief The requested column key is unavailable.
        missing_key
    };

    class DataTable;

    /// @brief Provides read-only access to one DataTable row.
    class DataTableRow final {
        std::shared_ptr<const std::vector<std::string>> keys_;
        std::vector<std::string> cells_;

        DataTableRow(
            std::shared_ptr<const std::vector<std::string>> keys,
            std::vector<std::string> cells
        ) noexcept
            : keys_(std::move(keys)), cells_(std::move(cells)) {}

        friend class DataTable;

    public:
        /// @brief Creates an empty row without keys or cells.
        DataTableRow() = default;

        /**
         * @brief Looks up a cell by its column key.
         * @param key Header value identifying the requested column.
         * @return A view of the cell value, or <code>table_error::missing_key</code> if the key is absent.
         */
        [[nodiscard]] jh::meta::expected<std::string_view, table_error>
        at(std::string_view key) const noexcept {
            if (!keys_) return jh::meta::unexpected(table_error::missing_key);

            for (std::size_t i = 0; i < keys_->size(); ++i) {
                if ((*keys_)[i] == key && i < cells_.size()) {
                    return std::string_view{cells_[i]};
                }
            }

            return jh::meta::unexpected(table_error::missing_key);
        }

        /**
         * @brief Returns a read-only view of this row's cell values in column order.
         * @return The row's cell values.
         */
        [[nodiscard]] std::span<const std::string> cells() const noexcept {
            return cells_;
        }
    };

    /// @brief Stores a header row and the DataTable's data rows.
    class DataTable final {
        std::shared_ptr<const std::vector<std::string>> keys_;
        std::vector<DataTableRow> rows_;

        explicit DataTable(
            std::shared_ptr<const std::vector<std::string>> keys
        ) noexcept
            : keys_(std::move(keys)) {}

    public:
        /// @brief Creates an empty table without headers or data rows.
        DataTable() = default;

        /**
         * @brief Builds a table from rows whose first row contains the column keys.
         * @param rows Header row followed by data rows; every data row must match the header width.
         * @return The table, or <code>table_error::missing_header</code> or <code>table_error::row_size_mismatch</code>.
         */
        [[nodiscard]] static jh::meta::expected<DataTable, table_error>
        from_rows(std::vector<std::vector<std::string>> rows) {
            if (rows.empty() || rows.front().empty()) {
                return jh::meta::unexpected(table_error::missing_header);
            }

            auto keys = std::make_shared<const std::vector<std::string>>(
                std::move(rows.front())
            );
            rows.erase(rows.begin());

            DataTable table{keys};
            table.rows_.reserve(rows.size());
            for (auto& cells : rows) {
                if (cells.size() != keys->size()) {
                    return jh::meta::unexpected(table_error::row_size_mismatch);
                }
                table.rows_.emplace_back(DataTableRow{keys, std::move(cells)});
            }

            return table;
        }

        /**
         * @brief Returns a read-only view of the column keys.
         * @return The header values, or an empty view for an empty table.
         */
        [[nodiscard]] std::span<const std::string> keys() const noexcept {
            if (!keys_) return {};
            return *keys_;
        }

        /**
         * @brief Returns a read-only view of the table's data rows.
         * @return Data rows in their original order.
         */
        [[nodiscard]] std::span<const DataTableRow> rows() const noexcept {
            return rows_;
        }
    };
}
