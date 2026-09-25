#pragma once

#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "jh/metax/expected.h"

namespace jh::test::cucumber {
    enum class table_error {
        missing_header,
        row_size_mismatch,
        missing_key
    };

    class DataTable;

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
        DataTableRow() = default;

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

        [[nodiscard]] std::span<const std::string> cells() const noexcept {
            return cells_;
        }
    };

    class DataTable final {
        std::shared_ptr<const std::vector<std::string>> keys_;
        std::vector<DataTableRow> rows_;

        explicit DataTable(
            std::shared_ptr<const std::vector<std::string>> keys
        ) noexcept
            : keys_(std::move(keys)) {}

    public:
        DataTable() = default;

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

        [[nodiscard]] std::span<const std::string> keys() const noexcept {
            if (!keys_) return {};
            return *keys_;
        }

        [[nodiscard]] std::span<const DataTableRow> rows() const noexcept {
            return rows_;
        }
    };
}
