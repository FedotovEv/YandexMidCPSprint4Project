#pragma once

#include <charconv>
#include <stdexcept>
#include <string>
#include <string_view>
#include <ranges>

namespace rv = std::ranges::views;
namespace rs = std::ranges;

#define ZERO_TOLERANCE 1E-7

struct TreeSitterOpt
{
    std::string_view tree_sitter_path;
    std::string_view tree_sitter_config_path;
    // Управление общим путевым префиксов всех исполняемых файлов.
    std::string_view common_files_path;
    bool use_common_files_path = false; // Общий префикс применяется.
};

inline TreeSitterOpt GetTriSitterOpt()
{
    char* tree_sitter_path = std::getenv("TREE-SITTER");
    if (!tree_sitter_path)
        throw std::runtime_error("Tree-sitter не найден");

    std::string* tree_sitter_path_p = new std::string(tree_sitter_path);

    TreeSitterOpt result;
    result.tree_sitter_path = *tree_sitter_path_p;
    result.tree_sitter_config_path = *tree_sitter_path_p;
    result.common_files_path = *tree_sitter_path_p;
    return result;
}

inline std::vector<std::string> ConstructFileNamesWithPath(const std::vector<std::string>& file_names, const TreeSitterOpt& tree_sitter_opt)
{
    auto file_path_names = file_names | rv::transform([&tree_sitter_opt](const std::string& filename) -> std::string
        {
            return std::string(tree_sitter_opt.tree_sitter_path) + '\\' + filename;
        });
    return rs::to<std::vector<std::string>>(file_path_names);
}

inline std::string_view TrimView(std::string_view value)
{
    std::string_view result = value;

    while (result.size() && std::isspace(result.front()))
        result.remove_prefix(1);

    while (result.size() && std::isspace(result.back()))
        result.remove_suffix(1);

    return result;
}

inline int ToInt(std::string_view value)
{
    std::string_view trimmed_val = TrimView(value);
    int result{};
    auto [parse_end_ptr, error_code] = std::from_chars(trimmed_val.data(), trimmed_val.data() + trimmed_val.size(), result);
    if (error_code != std::errc{} || parse_end_ptr != trimmed_val.data() + trimmed_val.size())
        throw std::invalid_argument("Cannot convert '" + std::string(value) + "' to integral");

    return result;
}
