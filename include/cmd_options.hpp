#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

#include <boost/program_options.hpp>
#include "utils.hpp"

namespace analyzer::cmd
{
    class ProgramOptions
    {
    public:
        ProgramOptions();
        ~ProgramOptions();

        bool Parse(int argc, char *argv[]);

        const std::vector<std::string>& GetFiles() const
        {
            return files_;
        }

        TreeSitterOpt GetTreeSitterOpt() const
        {
            return TreeSitterOpt
                {.tree_sitter_path = tree_sitter_path_, .tree_sitter_config_path = tree_sitter_config_path_,
                 .common_files_path = common_files_path_, .use_common_files_path = use_common_files_path_};
        }

    private:
        std::string tree_sitter_path_;
        std::string tree_sitter_config_path_;
        //
        std::string common_files_path_;
        bool use_common_files_path_ = false;
        std::vector<std::string> files_;
        //
        boost::program_options::options_description desc_;
    };
}  // namespace analyzer::cmd
