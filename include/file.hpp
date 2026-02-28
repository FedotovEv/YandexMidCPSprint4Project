#pragma once

#include <fstream>
#include <iostream>
#include <memory>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

namespace analyzer::file
{
    struct File
    {
        static inline const std::string tree_sitter_param_prefix = "parse --config-path";
        
        File(const std::string& filename, std::string_view tree_sitter_path = {}, std::string_view tree_sitter_config_path = {});
        std::string name;
        std::string ast;
        std::vector<std::string> source_lines;

    private:
        std::vector<std::string> ReadSourceFile(std::ifstream& file);
        std::string GetAst(const std::string& filename);
        // Дополнительные настройки для процедуры вызова tree-sitter'а.
        std::string tree_sitter_path_;
        std::string tree_sitter_config_path_;
    };
}  // namespace analyzer::file
